
#ifndef SORT_C
#define SORT_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIterator.h"
#include "MyDB_TableRecIteratorAlt.h"
#include "MyDB_TableReaderWriter.h"
#include "RecordComparator.h"
#include "Sorting.h"

using namespace std;

void appendHelper (MyDB_BufferManagerPtr buffer, 
				   MyDB_RecordPtr record,
                   vector <MyDB_PageReaderWriter> &list) {

	if (list.empty()) {
		// constructor for an anonymous page
		MyDB_PageReaderWriter pageRW (*buffer);
		list.push_back(pageRW);
	}

	MyDB_PageReaderWriter &lastPage = list.back();

	if (!lastPage.append(record)) {
		MyDB_PageReaderWriter pageRW (*buffer);
		list.push_back(pageRW);
	    list.back().append(record);
	}
};



// helper function.  Gets two iterators, leftIter and rightIter.  It is assumed that these are iterators over
// sorted lists of records.  This function then merges all of those records into a list of anonymous pages,
// and returns the list of anonymous pages to the caller.  The resulting list of anonymous pages is sorted.
// Comparisons are performed using comparator, lhs, rhs

vector <MyDB_PageReaderWriter> mergeIntoList (MyDB_BufferManagerPtr parent, 
											MyDB_RecordIteratorAltPtr leftIter,
        									MyDB_RecordIteratorAltPtr rightIter, 
        									function <bool ()> comparator, 
        									MyDB_RecordPtr lhs, 
        									MyDB_RecordPtr rhs) {
	
	vector <MyDB_PageReaderWriter> ret;

	bool lhsNotEnd = leftIter->advance();
	bool rhsNotEnd = rightIter->advance();

	if (lhsNotEnd) leftIter->getCurrent(lhs);
	if (rhsNotEnd) rightIter->getCurrent(rhs);
	
	while (lhsNotEnd || rhsNotEnd) {

		if (lhsNotEnd && rhsNotEnd) {
			if(comparator()){//lhs < rhs
				appendHelper(parent, lhs, ret);
				lhsNotEnd = leftIter->advance();
				if (lhsNotEnd) {
					leftIter->getCurrent(lhs);
				}
			}
			else {
				appendHelper(parent, rhs, ret);
				rhsNotEnd = rightIter->advance();
				if (rhsNotEnd){
					rightIter->getCurrent(rhs);
				}
			}
		} 
		else if (lhsNotEnd) {
			while(lhsNotEnd){
				leftIter->getCurrent(lhs);
				appendHelper(parent, lhs, ret);
				lhsNotEnd = leftIter->advance();
			}
		} 
		else{
			while(rhsNotEnd){
				rightIter->getCurrent(rhs);
				appendHelper(parent, rhs, ret);
				rhsNotEnd = rightIter->advance();
			}
		} 
	}
   
	return ret; 
} 



// performs a TPMMS of the table sortMe.  The results are written to sortIntoMe.  The run 
// size for the first phase of the TPMMS is given by runSize.  Comarisons are performed 
// using comparator, lhs, rhs	
void sort (int runSize, 
		  MyDB_TableReaderWriter &sortMe, 
	      MyDB_TableReaderWriter &sortIntoMe, 
	      function <bool ()> mycomparator, 
	      MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

	// get the number of pages in the file
    int pageNum = sortMe.getNumPages();

    //each internal vector is a sorted list from each run
    //vector < vector<MyDB_PageReaderWriter> > runList;
    //the list iterator for each run list
    vector <MyDB_RecordIteratorAltPtr> runListIter;


    //calculate the number of run
    int runNum = 0;
    if(pageNum % runSize == 0)
    	runNum = pageNum / runSize;
    else {
        runNum = pageNum / runSize +1;
    }
    for(int runIdx = 0; runIdx < runNum; runIdx++){
        //use the deque of vector 
        //deque <vector<MyDB_PageReaderWriter>> currentPageList;
    	deque <vector<MyDB_PageReaderWriter>> currentPageList;
    	
    	int currentRunSize = runSize;
    	//if the last run is not full, recalculate the size of the last run
    	if (runIdx == runNum - 1 && (pageNum % runSize != 0)){
         currentRunSize = pageNum % runSize;
    	}

        //RecordComparator myComp (mycomparator, lhs, rhs);

        //sort each individual page into anonymous page and insert page into run page list
        //insert the page list(vector which only have one page) to the external vector
    	for (int i = 0; i < currentRunSize; i++){
    		 vector<MyDB_PageReaderWriter> pageList;
    		 
    		 pageList.push_back(
    		 	// access the i^th page in this file
    		 	// sorts the contents of the page...
    		 	*(sortMe[runIdx * runSize + i].sort(mycomparator, lhs, rhs))
    		 	);

    		
    		currentPageList.push_back(pageList);

         }

        // ----------------
        // merge k sorted list
        while (currentPageList.size() > 1){
        	// gets an instance of an alternatie iterator over a list of pages
        	
			vector<MyDB_PageReaderWriter> merge2 = 
						mergeIntoList(sortMe.getBufferMgr(), 
										getIteratorAlt(currentPageList[0]),
										getIteratorAlt(currentPageList[1]), 
										mycomparator, lhs, rhs);

			currentPageList.push_back(merge2);
        	currentPageList.pop_front();
        	currentPageList.pop_front();
        }

        
        runListIter.push_back(getIteratorAlt(currentPageList.front()));
    }
	
    mergeIntoFile(sortIntoMe, runListIter,  mycomparator, lhs, rhs); 
} 

// accepts a list of iterators called mergeUs.  It is assumed that these are all iterators over sorted lists
// of records.  This function then merges all of those records and appends them to the file sortIntoMe.  If
// all of the iterators are over sorted lists of records, then all of the records appended onto the end of
// sortIntoMe will be sorted.  Comparisons are performed using comparator, lhs, rhs
void mergeIntoFile (MyDB_TableReaderWriter &sortIntoMe,
					vector <MyDB_RecordIteratorAltPtr> &mergeUs,
					function <bool ()> comparator,
					MyDB_RecordPtr lhs, 
					MyDB_RecordPtr rhs) {
	// first create a comparator for record
	RecordComparator recordComp (comparator, lhs, rhs);
	
	// create a comparator for record iterator
	// 这里的比较函数时大于，用于定义min heap
    auto recIterComp = [&recordComp]
    					(MyDB_RecordIteratorAltPtr left,
				  		MyDB_RecordIteratorAltPtr right) {
		
		return !recordComp(left->getCurrentPointer(), right->getCurrentPointer());
	};

	// create a priority queue for holding the records from mergeUs
	// C++ default: max heap
	priority_queue <MyDB_RecordIteratorAltPtr,
					vector <MyDB_RecordIteratorAltPtr>, 
					decltype(recIterComp)>
		minHeap (recIterComp, mergeUs);

	while (!minHeap.empty()) {
		// pull the iterator with the smallest current record
		MyDB_RecordIteratorAltPtr top = minHeap.top();
		minHeap.pop();

		top->getCurrent(lhs);
		
		sortIntoMe.append (lhs);

		if (top->advance ()) {
			minHeap.push (top);
		}
		
	}
}


#endif
