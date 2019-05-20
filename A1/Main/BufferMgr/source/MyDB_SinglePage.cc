//
// Created by 黄思奇 on 1/25/18.
//

#ifndef SINGLE_PAGE_C
#define SINGLE_PAGE_C

#include <memory>
#include <iostream>
#include <fcntl.h>
#include <zconf.h>
#include "../headers/MyDB_SinglePage.h"
#include "../headers/MyDB_BufferManager.h"

MyDB_SinglePageBase ::~MyDB_SinglePageBase() {

}

MyDB_SinglePageBase ::MyDB_SinglePageBase(MyDB_BufferManager& my_bufferManager,char * my_buffer,string my_storageLoc,long i):
        bufferManager(my_bufferManager),buffer(my_buffer),storageLoc(my_storageLoc),
        ith(i),pinned(false),anonymous(false),dirty(false)  {}

void MyDB_SinglePageBase ::pinPage() {
    pinned = true;
}

void MyDB_SinglePageBase ::unpinPage() {
    pinned = false;
}

void MyDB_SinglePageBase ::setnHandle(int i) {
    nHandle += i;
    if(nHandle == 0){
        pinned = false;
        if(anonymous){
            dirty = false;
            bufferManager.recycleAnonIdx(ith);
        }
    } else if(nHandle < 0){
        cout << "error: wrong counter of the handle "<<endl;
        exit(1);
    }
}


void MyDB_SinglePageBase ::setPosition(list <MyDB_SinglePagePtr>::iterator pos) {
    position = pos;
}

void MyDB_SinglePageBase::updateLRU() {
    bufferManager.updateLRUList(shared_from_this());
}


bool MyDB_SinglePageBase::getDirty(){
    return dirty;
}

string MyDB_SinglePageBase::getStorageLoc() {
    return storageLoc;
}

long MyDB_SinglePageBase::getIdx() {
    return ith;
}

char* MyDB_SinglePageBase::getBuffer() {
    return buffer;
}

void MyDB_SinglePageBase::readFile() {

    int file = open(storageLoc.c_str(), O_RDONLY | O_FSYNC);
    if (file >= 0) {
        if(lseek(file, bufferManager.getPageSize()*ith, SEEK_SET) >= 0) {
            ssize_t readNum = read(file, buffer, bufferManager.getPageSize());
            if(readNum<0){
                cout << "error: probably wrong with the buffer or the position of the file" << endl;
                cout << "errno: " << errno << endl;
                exit (1);
            }
        }
        close(file);
    } else{
        cout << "MyDB_SinglePageBase::readFile: fail to open the file"<<endl;
    }
    dirty = false;
}

void MyDB_SinglePageBase::setBuffer(char * my_buffer) {
    buffer = my_buffer;
}

void MyDB_SinglePageBase::setDirty(bool my_dirty) {
    dirty = my_dirty;
}

bool MyDB_SinglePageBase::isPinned(){
    return pinned;
}

list <MyDB_SinglePagePtr> :: iterator MyDB_SinglePageBase::getPosition(){
    return position;
}

void MyDB_SinglePageBase::setAnonymous(bool my_anon) {
    anonymous=my_anon;
}



MyDB_BufferManager& MyDB_SinglePageBase::getBufferManager(){
    return bufferManager;
}


#endif