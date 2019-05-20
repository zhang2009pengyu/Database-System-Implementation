//-----


#ifndef BPLUS_C
#define BPLUS_C

#include "MyDB_INRecord.h"
#include "MyDB_BPlusTreeReaderWriter.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_PageListIteratorSelfSortingAlt.h"
#include "RecordComparator.h"


//completed
MyDB_BPlusTreeReaderWriter :: MyDB_BPlusTreeReaderWriter (string orderOnAttName, MyDB_TablePtr forMe,
                                                          MyDB_BufferManagerPtr myBuffer) : MyDB_TableReaderWriter (forMe, myBuffer) {

    // find the ordering attribute
    auto res = forMe->getSchema ()->getAttByName (orderOnAttName);

    // remember information about the ordering attribute
    orderingAttType = res.second;
    whichAttIsOrdering = res.first;
    //rootLocation在append的时候初始化
}

//complted
MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter :: getSortedRangeIteratorAlt (MyDB_AttValPtr low, MyDB_AttValPtr high) {

    //a list of leaf page which have records fall in(low, high)
    vector<MyDB_PageReaderWriter> list;
    discoverPages(rootLocation, list, low, high);

    //used to sortInplace
    MyDB_RecordPtr lhsIn = getEmptyRecord();
    MyDB_RecordPtr rhsIn = getEmptyRecord();
    function<bool()> comparatorIn = buildComparator(lhsIn, rhsIn);

    //myRecIn is used to iterate the page
    MyDB_RecordPtr myRecIn = getEmptyRecord();

    //leaf record and internal record can actually compare!! (realized by buildComparator)
    MyDB_INRecordPtr lowBoundRec = getINRecord();
    lowBoundRec->setKey(low);
    MyDB_INRecordPtr highBoundRec = getINRecord();
    highBoundRec->setKey(high);

    //!low is (>=low) refer the def. of MyDB_PageListIteratorSelfSortingAlt
    function <bool()> lowComparatorIn = buildComparator(myRecIn, lowBoundRec); //myRecIn < lowBoundRec -> true
    //!high is (<=high)
    function <bool()> highComparatorIn = buildComparator(highBoundRec, myRecIn); //highBoundRec < myRecIn -> true

    MyDB_RecordIteratorAltPtr listIter = make_shared<MyDB_PageListIteratorSelfSortingAlt>(list, lhsIn, rhsIn, comparatorIn,
                                                                                          myRecIn, lowComparatorIn, highComparatorIn, true);
    return listIter;
}

//completed
MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter :: getRangeIteratorAlt (MyDB_AttValPtr low, MyDB_AttValPtr high) {

    //a list of leaf page which have records fall in(low, high)
    vector<MyDB_PageReaderWriter> list;
    discoverPages(rootLocation, list, low, high);

    //used to sortInplace
    MyDB_RecordPtr lhsIn = getEmptyRecord();
    MyDB_RecordPtr rhsIn = getEmptyRecord();
    function<bool()> comparatorIn = buildComparator(lhsIn, rhsIn);

    //myRecIn is used to iterate the page
    MyDB_RecordPtr myRecIn = getEmptyRecord();

    //leaf record and internal record can actually compare!! (realized by buildComparator)
    //build record from attribute low and high
    MyDB_INRecordPtr lowBoundRec = getINRecord();
    lowBoundRec->setKey(low);
    MyDB_INRecordPtr highBoundRec = getINRecord();
    highBoundRec->setKey(high);


    function <bool()> lowComparatorIn = buildComparator(myRecIn, lowBoundRec);
    function <bool()> highComparatorIn = buildComparator(highBoundRec, myRecIn);

    MyDB_RecordIteratorAltPtr listIter = make_shared<MyDB_PageListIteratorSelfSortingAlt>(list, lhsIn, rhsIn, comparatorIn,
                                                                                          myRecIn, lowComparatorIn, highComparatorIn, false);

    return listIter;

}


// The return value from this method is a boolean indicating whether the page pointed to by whichPage was at the leaf level
// discover到的pages放在list里面
bool MyDB_BPlusTreeReaderWriter :: discoverPages (int whichpage, vector <MyDB_PageReaderWriter> &list,
                                                  MyDB_AttValPtr low, MyDB_AttValPtr high) {

    //find the page
    MyDB_PageReaderWriter pageToSearch = (*this)[whichpage];

    // leaf page
    if (pageToSearch.getType () == MyDB_PageType :: RegularPage) {

        list.push_back (pageToSearch);
        return true;

    }
    else {//internal node

        // iterate through the various subtrees
        MyDB_RecordIteratorAltPtr currPtr = pageToSearch.getIteratorAlt ();

        // set up all of the comparisons that we need
        MyDB_INRecordPtr otherRec = getINRecord ();

        MyDB_INRecordPtr llow = getINRecord ();
        llow->setKey (low);
        MyDB_INRecordPtr hhigh = getINRecord ();
        hhigh->setKey (high);

        function <bool ()> comparatorLow = buildComparator (otherRec, llow);
        function <bool ()> comparatorHigh = buildComparator (hhigh, otherRec);

        // if the low bound and the high bound are both engaged, then we are discovering records
        bool lowInRange = false;
        bool highInRange = true;
        bool isLeafPage = false;
        while (currPtr->advance ()) {
            currPtr->getCurrent (otherRec);

            if (!comparatorLow ()){//llow <= otherRec
                lowInRange = true;
            }


            if(isLeafPage){
                if(lowInRange && highInRange){
                    list.push_back ((*this)[otherRec->getPtr ()]);
                }
            }
            else{//not laef page
                if(lowInRange && highInRange){
                    isLeafPage = discoverPages (otherRec->getPtr (), list, low, high);
                }
            }


            if (comparatorHigh ()){
                highInRange = false;
            }

        }
        return false;
    }
}

//complete
void MyDB_BPlusTreeReaderWriter :: append (MyDB_RecordPtr appendMe) {

    //理解下面的代码非常有用：
    //non leaf page插入的是INRecordPtr; leaf page插入的是RecordPtr
    if (getNumPages () <= 1) {

        // the root is at page location zero
        MyDB_PageReaderWriter root = (*this)[0];
        rootLocation = 0;//set rootLocation to 0

        // get an internal node record that has a pointer to page 1
        MyDB_INRecordPtr internalNodeRec = getINRecord ();
        internalNodeRec->setPtr (1);//是root放的第1个record
        getTable ()->setLastPage (1);

        // add that internal node record in
        root.clear ();
        root.append (internalNodeRec);//non leaf page插入的是INRecordPtr
        root.setType (MyDB_PageType :: DirectoryPage);

        // and add the new record to the leaf
        MyDB_PageReaderWriter leaf1 = (*this)[1];
        leaf1.clear ();
        leaf1.setType (MyDB_PageType :: RegularPage);
        leaf1.append (appendMe);
        return;
    } //get
    else {

        // append the record into the tree
        auto res = append (rootLocation, appendMe);

        // see if the root split
        if (res != nullptr) {
//
//            // add another page to the file

            lastPage = make_shared<MyDB_PageReaderWriter>(*this, forMe->lastPage()+1);
            lastPage->clear();
            lastPage->append(res);


            //get an infinity internal record
            MyDB_INRecordPtr infinityRecord = getINRecord();
            //set the ptr, point to the old root
            infinityRecord->setPtr(rootLocation);


            //append the infinity record
            lastPage->append(infinityRecord);
            lastPage->setType(MyDB_PageType::DirectoryPage);

            //set last to table
            forMe->setLastPage (forMe->lastPage () + 1);
            //set new root location
            rootLocation = forMe->lastPage();

            forMe->setRootLocation(rootLocation);
        }
        else{
            return;
        }

    }

}


//complete
//(  ((char *) temp) + sizeof (size_t))  这是一个地址
//(size_t *) (  ((char *) temp) + sizeof (size_t)) 这还是同一个地址，区别在于指针的类型是size_t*
//*((size_t *) (((char *) temp) + sizeof (size_t))) 读这个地址的值，作为size_t这个类型来读。
MyDB_RecordPtr MyDB_BPlusTreeReaderWriter :: split (MyDB_PageReaderWriter splitMe, MyDB_RecordPtr appendMe) {
    // if the split page is a leaf page
    if(splitMe.getType() == MyDB_PageType::RegularPage){

        MyDB_RecordPtr temp1 = getEmptyRecord();
        MyDB_RecordPtr temp2 = getEmptyRecord();
        function<bool()> myComp = buildComparator(temp1, temp2);
        //first sort inside the leaf page
        splitMe.sortInPlace(myComp, temp1, temp2);



        //put all the RecordPtr to the tempRecord List sorted
        vector <MyDB_RecordPtr> tempRecordList;
        MyDB_RecordIteratorAltPtr myIter = splitMe.getIteratorAlt();
        bool alreadyAdd = false;
        while(myIter->advance()){
            MyDB_RecordPtr temp = getEmptyRecord();
            //record number is the capacity of leaf page+1
            function<bool()> CompforList = buildComparator(appendMe, temp);//appendMe < temp


            myIter->getCurrent(temp);

            if(!alreadyAdd && CompforList()){
                tempRecordList.push_back(appendMe);
                alreadyAdd = true;
            }

            tempRecordList.push_back(temp);
        }
        if(!alreadyAdd){
            tempRecordList.push_back(appendMe);
        }

        //-----------
        //create a new page in the end of the file
        //now have two page, splitMe and lastpage
        lastPage = make_shared<MyDB_PageReaderWriter>(*this, forMe->lastPage()+1);
        lastPage->clear();
        //set last to table
        forMe->setLastPage (forMe->lastPage () + 1);
        lastPage->setType(MyDB_PageType::RegularPage);

        //midNum
        int midNum = tempRecordList.size() / 2 + tempRecordList.size() % 2;

        //put the small record in the new page
        for(int i = 0; i < midNum; i++){
            lastPage->append(tempRecordList[i]);
        }

        //put the large record in the splitme page
        splitMe.clear();
        for(int i = midNum; i < tempRecordList.size(); i++){
            splitMe.append(tempRecordList[i]);
        }

        //create a new internal record
        MyDB_INRecordPtr retRecord = getINRecord();
        retRecord->setPtr(forMe->lastPage ());
        retRecord->setKey(getKey(tempRecordList[midNum-1]));//biggest on the new page
        return retRecord;

    }

    else{//internal page


        MyDB_INRecordPtr temp = getINRecord ();

        //store all the sorted records
        //vector <MyDB_RecordPtr> tempRecordList = MergeIntoList(splitMe, appendMe, temp, myComp2);

        vector <MyDB_RecordPtr> tempRecordList;
        MyDB_RecordIteratorAltPtr myIter = splitMe.getIteratorAlt();

        bool alreadyAdd = false;
        while(myIter->advance()){
            MyDB_INRecordPtr temp = getINRecord();
            //record number is the capacity of leaf page+1
            function<bool()> CompforList = buildComparator(appendMe, temp);//appendMe < temp


            myIter->getCurrent(temp);

            if(!alreadyAdd && CompforList()){
                tempRecordList.push_back(appendMe);
                alreadyAdd = true;
            }

            tempRecordList.push_back(temp);
        }
        if(!alreadyAdd){
            tempRecordList.push_back(appendMe);
        }
        //-------
        //create a new page in the end of the file
        //now have two page, splitMe and lastpage
        lastPage = make_shared<MyDB_PageReaderWriter>(*this, forMe->lastPage() + 1);
        lastPage->clear();
        //set last to table
        forMe->setLastPage (forMe->lastPage () + 1);
        lastPage->setType(MyDB_PageType::DirectoryPage);


        int midNum = tempRecordList.size() / 2 + tempRecordList.size() % 2;

        //put the small record in the new page(not the mid record)

        for(int i = 0; i < midNum - 1; i++){
            lastPage->append(tempRecordList[i]);
        }

        //put the large page in the old page
        splitMe.clear();
        splitMe.setType(MyDB_PageType::DirectoryPage);
        for(int i = midNum; i < tempRecordList.size(); i++){
            splitMe.append(tempRecordList[i]);
        }


        //get the medium internal record
        MyDB_INRecordPtr retValue = static_pointer_cast<MyDB_INRecord>(tempRecordList[midNum - 1]);


        //create an infinity record for the new page
        MyDB_INRecordPtr newRecord = getINRecord();
        //new page's infinity record's ptr is the medium key's ptr
        newRecord->setPtr(retValue->getPtr());

        lastPage->append(newRecord);

        //point to the new page
        retValue->setPtr(forMe->lastPage ());

        return retValue;
    }
}

//completed
//代码看懂了
//插入成功就返回nullptr
//插入不成功就返回new page的最后一个key
MyDB_RecordPtr MyDB_BPlusTreeReaderWriter :: append (int whichPage, MyDB_RecordPtr appendMe) {
    MyDB_PageReaderWriter pageToAddTo = (*this)[whichPage];

    if(pageToAddTo.getType() == MyDB_PageType::RegularPage){

        MyDB_PageReaderWriter leafpage = (*this)[whichPage];
        //if the leaf page if already full, split the leaf page
        if (!leafpage.append(appendMe)){
            //refer the example on database implementation p642
            return split(leafpage, appendMe);
        }
        else {//insert to leaf node successfully
            return nullptr;
        }
    }//get

    else{//for directory page
        MyDB_RecordIteratorAltPtr myIter = pageToAddTo.getIteratorAlt();
        MyDB_INRecordPtr temp = getINRecord();
        function<bool()> myComp = buildComparator(temp,appendMe);

        while(myIter->advance()){
            myIter->getCurrent(temp);

            //temp >= appendMe
            if(!myComp()){
                break;
            }
        }

        //the next level is internal,recusively find

        MyDB_RecordPtr InRecord;
        InRecord = append(temp->getPtr(), appendMe);


        //the lower level has append successfully
        if(InRecord == nullptr){
            return nullptr;
        }

//        if the lower level cause a split, then will try to append the (PE, ptr)
//        (PE, ptr) is from the lower level's split return
        else{

            if(!pageToAddTo.append(InRecord)){

                return split(pageToAddTo,InRecord);
            }

//            append the (PE, ptr)successfully
//            need to sort the internal page
            else{
                MyDB_RecordPtr recordPtr1 = getINRecord();
                MyDB_RecordPtr recordPtr2 = getINRecord();
                function<bool()> myComp = buildComparator(recordPtr1, recordPtr2);
                //first sort the internal page
                pageToAddTo.sortInPlace(myComp, recordPtr1, recordPtr2);
                return nullptr;
            }
        }
    }
}


//completed
//just for test
void MyDB_BPlusTreeReaderWriter :: printTree () {
    queue <MyDB_PageReaderWriter> pageQueue;
    queue <int> pageNumQueue;

    pageQueue.push((*this)[rootLocation]);
    pageNumQueue.push(rootLocation);

    while(!pageQueue.empty()){

        MyDB_PageReaderWriter currentPage = pageQueue.front();
        pageQueue.pop();

        int curPageNum = pageNumQueue.front();
        pageNumQueue.pop();

        if(currentPage.getType() == MyDB_PageType::DirectoryPage){

            cout << "internal:" << "#" << curPageNum << ":" << endl;
            //iterate over the directory page
            MyDB_INRecordPtr temp = getINRecord();
            MyDB_RecordIteratorAltPtr currentIter = currentPage.getIteratorAlt();

            while(currentIter->advance()){

                currentIter->getCurrent(temp);

                cout<<"*"<<(MyDB_RecordPtr)temp << endl;

                pageQueue.push((*this)[temp->getPtr()]);
                pageNumQueue.push(temp->getPtr());
            }
            cout << endl;
        }
        else{

            cout<<"leaf:" << "#" << curPageNum << ":" << endl;
            //iterate over the leaf page
            MyDB_RecordPtr leaftemp = getEmptyRecord();
            MyDB_RecordIteratorAltPtr leafIter = currentPage.getIteratorAlt();

            while(leafIter->advance()){
                leafIter->getCurrent(leaftemp);
                cout<<"*"<<leaftemp<<endl;
            }
            cout << endl;
        }
    }
}


//implemented by professor
MyDB_INRecordPtr MyDB_BPlusTreeReaderWriter :: getINRecord () {
    return make_shared<MyDB_INRecord> (orderingAttType->createAttMax ());
}

//implemented by professor
//it obtains the value that the record is sorted on
MyDB_AttValPtr MyDB_BPlusTreeReaderWriter :: getKey (MyDB_RecordPtr fromMe) {

    // in this case, got an IN record
    if (fromMe->getSchema () == nullptr)
        return fromMe->getAtt (0)->getCopy ();

        // in this case, got a data record
        //whichAttIsOrdering是一个int
    else
        return fromMe->getAtt (whichAttIsOrdering)->getCopy ();
}

//provided by professor
function <bool ()>  MyDB_BPlusTreeReaderWriter :: buildComparator (MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

    MyDB_AttValPtr lhAtt, rhAtt;
    //这个看懂了
    // in this case, the LHS is an IN record
    if (lhs->getSchema () == nullptr) {

        lhAtt = lhs->getAtt (0);

        // here, it is a regular data record
    } else {
        lhAtt = lhs->getAtt (whichAttIsOrdering);
    }

    //对rhs做同样的判断
    // in this case, the LHS is an IN record
    if (rhs->getSchema () == nullptr) {

        rhAtt = rhs->getAtt (0);

        // here, it is a regular data record
    } else {
        rhAtt = rhs->getAtt (whichAttIsOrdering);
    }

    // now, build the comparison lambda and return
    if (orderingAttType->promotableToInt ()) {
        return [lhAtt, rhAtt] {return lhAtt->toInt () < rhAtt->toInt ();};
    }
    else if (orderingAttType->promotableToDouble ()) {
        return [lhAtt, rhAtt] {return lhAtt->toDouble () < rhAtt->toDouble ();};
    }
    else if (orderingAttType->promotableToString ()) {
        return [lhAtt, rhAtt] {return lhAtt->toString () < rhAtt->toString ();};
    }
    else {
        cout << "This is bad... cannot do anything with the >.\n";
        exit (1);
    }
}


#endif
