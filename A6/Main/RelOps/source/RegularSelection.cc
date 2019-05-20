#ifndef REG_SELECTION_C
#define REG_SELECTION_C

#include "MyDB_Record.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "RegularSelection.h"
#include "../../Record/headers/MyDB_Record.h"

using namespace std;

RegularSelection :: RegularSelection (MyDB_TableReaderWriterPtr inputTable,
                                      MyDB_TableReaderWriterPtr outputTable,
                                      string selectionPredicate,
                                      vector <string> projections){

    this->inputTable = inputTable;
    this->outputTable = outputTable;
    this->selectionPredicate = selectionPredicate;
    this->projections = projections;
}

void  RegularSelection :: run(){
    MyDB_RecordIteratorAltPtr inputIter = inputTable->getIteratorAlt();
    MyDB_RecordPtr inputRec = inputTable->getEmptyRecord();
    MyDB_RecordPtr outputRec = outputTable->getEmptyRecord();

    // now get the predicate
    func selectionPred = inputRec->compileComputation(selectionPredicate);

    // and get the final set of computatoins that will be used to buld the output record
    vector <func> finalComputations;
    for (string s : projections) {
        finalComputations.push_back (inputRec->compileComputation (s));
    }

    //iterate over the input table
    while(inputIter->advance()){
        inputIter->getCurrent(inputRec);

        if(selectionPred()->toBool()){
            int i = 0;
            for(auto &f : finalComputations){
                outputRec->getAtt (i++)->set (f());
            }
            outputRec->recordContentHasChanged ();
            outputTable->append (outputRec);
        }
    }

}
#endif