
#ifndef BPLUS_SELECTION_C                                        
#define BPLUS_SELECTION_C

#include "BPlusSelection.h"

BPlusSelection :: BPlusSelection (MyDB_BPlusTreeReaderWriterPtr input,
                                  MyDB_TableReaderWriterPtr output,
                                  MyDB_AttValPtr low,
                                  MyDB_AttValPtr high,
                                  string selectionPredicate,
                                  vector <string> projections){
    this->input = input;
    this->output = output;
    this->low = low;
    this->high = high;
    this->selectionPredicate = selectionPredicate;
    this->projections = projections;

}

void BPlusSelection :: run () {

    // get the range use BPlusTree
    MyDB_RecordIteratorAltPtr iter = input->getRangeIteratorAlt (low, high);
    MyDB_RecordPtr outputRec = output->getEmptyRecord ();
    MyDB_RecordPtr inputRec = input->getEmptyRecord ();

    func selectionPred = inputRec->compileComputation (selectionPredicate);

    vector <func> finalComputations;
    for (string s : projections) {
        finalComputations.push_back (inputRec->compileComputation (s));
    }

    while (iter->advance()) {
        // get the current record
        iter->getCurrent (inputRec);

        // check if it is accepted by the selection predicate
        if (selectionPred ()->toBool ()) {
            int i = 0;
            // projections
            for (auto& f : finalComputations ) {
                outputRec->getAtt (i++)->set (f ());
            }

            // notify the record that the content has changed
            outputRec->recordContentHasChanged ();
            output->append(outputRec);
        }
    }
}

#endif
