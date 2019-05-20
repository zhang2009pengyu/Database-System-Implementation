#include <stdio.h>
#include "MyDB_Record.h"
#include "MyDB_PageReaderWriter.h"
#include "SortMergeJoin.h"

#include "Sorting.h"


using namespace std;

SortMergeJoin :: SortMergeJoin (MyDB_TableReaderWriterPtr leftInput, MyDB_TableReaderWriterPtr rightInput,
                                MyDB_TableReaderWriterPtr output, string finalSelectionPredicate,
                                vector <string> projections,
                                pair <string, string> equalityCheck, string leftSelectionPredicate,
                                string rightSelectionPredicate){
    this->leftInput = leftInput;
    this->rightInput = rightInput;
    this->output = output;
    this->finalSelectionPredicate = finalSelectionPredicate;
    this->projections = projections;
    this->equalityCheck = equalityCheck;
    this->leftSelectionPredicate = leftSelectionPredicate;
    this->rightSelectionPredicate = rightSelectionPredicate;
}

// execute the join
void SortMergeJoin:: run (){

    MyDB_RecordPtr leftInputRec = leftInput->getEmptyRecord ();
    MyDB_RecordPtr rightInputRec = rightInput->getEmptyRecord ();


    MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();
    for (auto p : leftInput->getTable ()->getSchema ()->getAtts ())
        mySchemaOut->appendAtt (p);
    for (auto p : rightInput->getTable ()->getSchema ()->getAtts ())
        mySchemaOut->appendAtt (p);


    MyDB_RecordPtr combinedRec = make_shared <MyDB_Record> (mySchemaOut);


    combinedRec->buildFrom (leftInputRec, rightInputRec);

    // get some compare func
    func leftSmaller = combinedRec->compileComputation (" < (" + equalityCheck.first + ", " + equalityCheck.second + ")");
    func rightSmaller = combinedRec->compileComputation (" > (" + equalityCheck.first + ", " + equalityCheck.second + ")");
    func areEqual = combinedRec->compileComputation (" == (" + equalityCheck.first + ", " + equalityCheck.second + ")");



    func leftEqualities;
    leftEqualities = leftInputRec->compileComputation (equalityCheck.first);


    func rightEqualities;
    rightEqualities = rightInputRec->compileComputation (equalityCheck.second);

    //-----------Sort phase--------
    MyDB_TablePtr leftTable = make_shared <MyDB_Table> ("leftSorted", "leftSorted.bin", leftInput->getTable ()->getSchema ());
    MyDB_BufferManagerPtr leftMgr = leftInput->getBufferMgr();
    MyDB_TableReaderWriter leftSorted (leftTable, leftMgr);
    MyDB_RecordPtr lhsL = leftInput->getEmptyRecord();
    MyDB_RecordPtr rhsL = leftInput->getEmptyRecord();
    function <bool ()> f1 = buildRecordComparator(lhsL, rhsL, equalityCheck.first);
    sort (64, *leftInput, leftSorted, f1, lhsL, rhsL);


    MyDB_TablePtr rightTable = make_shared <MyDB_Table> ("rightSorted", "rightSorted.bin", rightInput->getTable ()->getSchema ());
    MyDB_BufferManagerPtr rightMgr = rightInput->getBufferMgr();
    MyDB_TableReaderWriter rightSorted (rightTable, rightMgr);
    MyDB_RecordPtr lhsR = rightInput->getEmptyRecord();
    MyDB_RecordPtr rhsR = rightInput->getEmptyRecord();
    function <bool ()> f2 = buildRecordComparator(lhsR, rhsR, equalityCheck.second);
    sort (64, *rightInput, rightSorted, f2, lhsR, rhsR);


    //---------Merge phase---------

    MyDB_RecordIteratorAltPtr iterL = leftSorted.getIteratorAlt ();
    MyDB_RecordPtr recL = leftSorted.getEmptyRecord ();
    MyDB_RecordIteratorAltPtr iterR = rightSorted.getIteratorAlt ();
    MyDB_RecordPtr recR = rightSorted.getEmptyRecord ();

    vector<MyDB_PageReaderWriter> leftBox;
    vector<MyDB_PageReaderWriter> rightBox;
    bool leftMove = true;
    bool rightMove = true;
    bool leftEnd = false;
    bool rightEnd = false;
    while (!leftEnd || !rightEnd){
        iterL->getCurrent(recL);
        iterR->getCurrent(recR);

        if(leftBox.size() == 0 || rightBox.size() == 0){

            if (leftMove) {
                // now get the predicate
                func leftPred = recL->compileComputation (leftSelectionPredicate);

                int checkLeft = checkSingleAcceptance(leftPred,iterL,recL);

                if(checkLeft == 1){
                    // no more records
                    leftEnd = true;
                    leftMove = false;
                    continue;
                }
                else if(checkLeft == 2){
                    leftMove = true;
                    continue;
                }
                else {
                    leftMove = false;
                }
            }
            if (rightMove) {
                // now get the predicate
                func rightPred = recR->compileComputation (rightSelectionPredicate);
                int checkRight=checkSingleAcceptance(rightPred,iterR,recR);
                if(checkRight==1){
                    // no more records
                    rightEnd = true;
                    rightMove = false;
                    continue;
                }

                else if(checkRight == 2){
                    // not accepted
                    rightMove = true;
                    continue;
                }

                else {
                    rightMove = false;
                }
            }
            // get the combined record
            MyDB_RecordPtr combinedRec = make_shared <MyDB_Record> (mySchemaOut);
            // now, get the final predicate over it

            // and make it a composite of the two input records
            combinedRec->buildFrom (recL, recR);

            // get some compare func
            func leftSmaller = combinedRec->compileComputation (" < (" + equalityCheck.first + ", " + equalityCheck.second + ")");
            func rightSmaller = combinedRec->compileComputation (" > (" + equalityCheck.first + ", " + equalityCheck.second + ")");
            func areEqual = combinedRec->compileComputation (" == (" + equalityCheck.first + ", " + equalityCheck.second + ")");
            if(leftSmaller ()->toBool ()){
                if(!iterL->advance()){
//                    cout << "left no more records\n";
                    leftEnd = true;
                    continue;
                }
                else{

                    leftMove = true;
                    rightMove = false;
                    continue;
                }
            }
            else if(rightSmaller ()->toBool ()){
                if(!iterR->advance()){

                    rightEnd = true;
                    rightMove = false;
                    continue;
                }
                else{

                    leftMove = false;
                    rightMove = true;
                    continue;
                }
            }
            else if (areEqual() ->toBool()){

                MyDB_PageReaderWriter prwL(*leftInput->getBufferMgr());
                MyDB_PageReaderWriter prwR (*rightInput->getBufferMgr());
                prwL.append(recL);
                prwR.append(recR);
                leftBox.push_back(prwL);
                rightBox.push_back(prwR);
                if(!iterL->advance()){
                    leftEnd = true;
                    leftMove = false;
                }
                else{
                    leftMove=true;
                }

                if(!iterR->advance()){
                    rightEnd= true;
                    rightMove=false;
                }
                else{
                    rightMove=true;
                }

            }

        }
        else{

            int stillNum = 0;
            if (leftMove) {
                //int rightNextState=nextState(equalityCheck.second, rightBox,iterR,recR,rightPred);
                MyDB_RecordPtr temp = leftSorted.getEmptyRecord ();
                MyDB_RecordIteratorAltPtr myIter = leftBox.front().getIteratorAlt ();
                myIter->getCurrent (temp);


                function <bool ()> f1 = buildRecordComparator(temp, recL, equalityCheck.first);
                function <bool ()> f2 = buildRecordComparator(recL, temp, equalityCheck.first);
                if(!f1() && !f2()){

                    // now get the predicate
                    func leftPred = recL->compileComputation (leftSelectionPredicate);
                    int check = checkSingleAcceptance(leftPred,iterL,recL);
                    if(check == 1){
                        leftEnd = true;
                        leftMove = false;;
                    }

                    else if(check == 2){
                        continue;
                    }
                    else {

                        if (!leftBox.back().append(recL)) {
                            MyDB_PageReaderWriter newPage(*leftInput->getBufferMgr());
                            newPage.append(recL);
                            leftBox.push_back(newPage);
                        }
                        if(!iterL->advance()){
                            leftEnd = true;
                            leftMove = false;
                        }

                    }
                }

                else{

                    stillNum++;
                    leftMove = false;
                }
            }
            if (rightMove) {

                //int rightNextState=nextState(equalityCheck.second, rightBox,iterR,recR,rightPred);
                MyDB_RecordPtr temp = rightSorted.getEmptyRecord ();
                MyDB_RecordIteratorAltPtr myIter = rightBox.front().getIteratorAlt ();
                myIter->getCurrent (temp);

                function <bool ()> f1 = buildRecordComparator(temp, recR, equalityCheck.second);
                function <bool ()> f2 = buildRecordComparator(recR, temp, equalityCheck.second);
                if(!f1() && !f2()){
                    // now get the predicate
                    func rightPred = recR->compileComputation (rightSelectionPredicate);
                    int check = checkSingleAcceptance(rightPred,iterR,recR);
                    if(check == 1){
                        rightEnd = true;
                        rightMove = false;
                    }

                    else if(check == 2){
                        continue;
                    }

                    else {
                        if (!rightBox.back().append(recR)) {
                            MyDB_PageReaderWriter newPage (*rightInput->getBufferMgr());
                            newPage.append(recR);
                            rightBox.push_back(newPage);
                        }
                        if(!iterR->advance()){
                            rightEnd = true;
                            rightMove = false;
                        }
                    }
                }
                else{
                    stillNum++;
                    rightMove = false;
                }
            }

            if(!rightMove && !leftMove){
                MyDB_RecordPtr recl = leftSorted.getEmptyRecord ();
                MyDB_RecordPtr recr = rightSorted.getEmptyRecord ();

                mergeRecs(recl, recr, leftBox, rightBox, output,mySchemaOut);
                leftBox.clear();
                rightBox.clear();

                leftMove = true;
                rightMove = true;
            }
        }
    }


}

int SortMergeJoin ::checkSingleAcceptance(func pred, MyDB_RecordIteratorAltPtr iter, MyDB_RecordPtr rec) {

    iter->getCurrent(rec);

    if (!pred ()->toBool ()) {
        if (!iter->advance()) {
            return 1;//end of the join
        }
        else {
            iter->getCurrent(rec);
            return 2;// continue
        }
    }

    else {
        return 3;//keep on
    }
}



void SortMergeJoin:: mergeRecs (MyDB_RecordPtr leftRec, MyDB_RecordPtr rightRec, vector<MyDB_PageReaderWriter> left, vector<MyDB_PageReaderWriter> right, MyDB_TableReaderWriterPtr output,MyDB_SchemaPtr mySchemaOut ){
    MyDB_RecordIteratorAltPtr iterL = getIteratorAlt(left);

    while(iterL->advance()){
        iterL->getCurrent(leftRec);

        MyDB_RecordPtr combinedRec = make_shared <MyDB_Record> (mySchemaOut);
        combinedRec->buildFrom (leftRec, rightRec);
        // now, get the final predicate over it
        func finalPredicate = combinedRec->compileComputation (this->finalSelectionPredicate);

        // and get the final set of computatoins that will be used to buld the output record
        vector <func> finalComputations;
        for (string s : this->projections) {
            finalComputations.push_back (combinedRec->compileComputation (s));
        }
        MyDB_RecordIteratorAltPtr iterR = getIteratorAlt(right);

        while(iterR->advance()){
            iterR->getCurrent(rightRec);
            MyDB_RecordPtr outputRec = output->getEmptyRecord ();
            if (finalPredicate ()->toBool ()) {

                // run all of the computations
                int i = 0;
                for (auto f : finalComputations) {
                    outputRec->getAtt (i++)->set (f());
                }

                outputRec->recordContentHasChanged ();
                output->append (outputRec);
            }

        }
    }
}