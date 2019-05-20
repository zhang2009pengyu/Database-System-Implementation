
#ifndef AGG_CC
#define AGG_CC

#include "MyDB_Record.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "Aggregate.h"
#include <unordered_map>


using namespace std;

Aggregate :: Aggregate (MyDB_TableReaderWriterPtr input,
                        MyDB_TableReaderWriterPtr output,
                        vector <pair <MyDB_AggType, string>> aggsToCompute,
                        vector <string> groupings,
                        string selectionPredicate){
    this->input = input;
    this->output = output;
    this->aggsToCompute = aggsToCompute;
    this->groupings = groupings;
    this->selectionPredicate = selectionPredicate;

}



void Aggregate :: run () {
    // create the schema for the combined records (refer scan join)
    MyDB_SchemaPtr combinedSchema = make_shared <MyDB_Schema> ();
    for (auto &a : input->getTable ()->getSchema ()->getAtts ())
        combinedSchema->appendAtt (a);

    // create a schema for the aggregate records... this is all grouping atts,
    // followed by all aggregate atts, followed by the count att
    MyDB_SchemaPtr aggSchema = make_shared <MyDB_Schema> ();
    int i = 0;
    int numGroups = groupings.size ();

//    group number from 0
//    agg number from 0

    for (auto &a : output->getTable ()->getSchema ()->getAtts ()) {
        if (i < numGroups)
            aggSchema->appendAtt (make_pair ("MyDB_GroupAtt" + to_string (i++), a.second));
        else
            aggSchema->appendAtt (make_pair ("MyDB_AggAtt" + to_string (i++ - numGroups), a.second));
    }
    aggSchema->appendAtt (make_pair ("MyDB_CntAtt", make_shared <MyDB_IntAttType> ()));



    for (auto &a : aggSchema->getAtts ())
        combinedSchema->appendAtt (a);

//-----------------------------

    // now, get an intput rec, an agg rec, and a combined rec
    MyDB_RecordPtr inputRec = input->getEmptyRecord ();
    MyDB_RecordPtr aggRec = make_shared <MyDB_Record> (aggSchema);
    MyDB_RecordPtr combinedRec = make_shared <MyDB_Record> (combinedSchema);
    combinedRec->buildFrom (inputRec, aggRec);

    // this is the current page where we are writing aggregate records
    MyDB_PageReaderWriter lastPage (true, *(input->getBufferMgr ()));



//--------------------------------

//    key: hash_value of grouping keys
//    value: combined records
    unordered_map <size_t, vector <void *>> myHash;

    // get the various functions whose output we'll hash
//    get
    vector <func> groupingFuncs;
    for (auto &g : groupings) {
        groupingFuncs.push_back (inputRec->compileComputation (g));
    }



    // and this will verify that each of the groupings match up
    func checkGroups;
    string groupCheck;
    i = 0;

    // in case there is not a grouping...
    groupCheck = "bool[true]";

    for (auto &s : groupings) {
        string curClause = "== (" + s + ", [MyDB_GroupAtt" + to_string (i) + "])";
        if (i == 0) {
            groupCheck = curClause;
        }
        else {
            groupCheck = "&& (" + groupCheck + ", " + curClause + ")";
        }
        i++;
    }
    checkGroups = combinedRec->compileComputation(groupCheck);


//    ---------------------

    // this will compute each of the aggregates for updating the aggregate record
    vector <func> aggComps;

    // this will compute the final aggregate value for each output record
    vector <func> finalAggComps;

    i = 0;
    for (auto &s : aggsToCompute) {
        switch (s.first){
            case MyDB_AggType::avg:
                aggComps.push_back (combinedRec->compileComputation
                        ("+ (" + s.second + ", [MyDB_AggAtt" + to_string (i) + "])"));
                finalAggComps.push_back (combinedRec->compileComputation
                        ("/ ([MyDB_AggAtt" + to_string (i++) + "], [MyDB_CntAtt])"));
                break;

            case MyDB_AggType::sum:
                aggComps.push_back (combinedRec->compileComputation
                        ("+ (" + s.second + ", [MyDB_AggAtt" + to_string (i) + "])"));
                finalAggComps.push_back (combinedRec->compileComputation
                        ("[MyDB_AggAtt" + to_string (i++) + "]"));
                break;

            case MyDB_AggType::cnt:
                aggComps.push_back (combinedRec->compileComputation
                        ("+ ( int[1], [MyDB_AggAtt" + to_string (i) + "])"));
                finalAggComps.push_back (combinedRec->compileComputation
                        ("[MyDB_AggAtt" + to_string (i++) + "]"));

                break;
            default:
                finalAggComps.push_back (combinedRec->compileComputation
                        ("[MyDB_AggAtt" + to_string (i++) + "]"));

        }


    }
    aggComps.push_back (combinedRec->compileComputation ("+ ( int[1], [MyDB_CntAtt])"));

//    -----------------------------------------------
    // and this runs the selection on the input records

    // this is the list all of the pages used to store aggregate records
    vector <MyDB_PageReaderWriter> allPages;
    allPages.push_back (lastPage);

    func inputPred = inputRec->compileComputation (selectionPredicate);


    // at this point, we are ready to go!!
    MyDB_RecordIteratorPtr myIter = input->getIterator (inputRec);
    MyDB_AttValPtr zero = make_shared <MyDB_IntAttVal> ();
    while (myIter->hasNext ()) {
        myIter->getNext ();


        if (inputPred ()->toBool ()) {
            // hash the current record
            size_t hashVal = 0;
            for (auto &f : groupingFuncs) {
                hashVal ^= f ()->hash ();
            }


            // if there is a match, then get the list of matches
            vector <void *> &potentialMatches = myHash [hashVal];
//            above if part, reference to scanjoin

//          next 4 lines refer to scanjoin
            void *loc = nullptr;
            for (auto &v : potentialMatches) {

                aggRec->fromBinary (v);

                // check to see if it matches
                if (checkGroups ()->toBool ()) {
                    loc = v;
                    break;
                }
            }

            // no match
            if (loc == nullptr) {

                // set up the record...
                i = 0;
                for (auto &f : groupingFuncs) {
                    aggRec->getAtt (i++)->set (f ());
                }
                for (int j = 0; j < aggComps.size (); j++) {
                    aggRec->getAtt (i++)->set (zero);
                }
            }

            // update each of the aggregates
            i = 0;
            for (auto &f : aggComps) {
                aggRec->getAtt (numGroups + i++)->set (f ());
            }

            // no match -> write to a new location...
            aggRec->recordContentHasChanged ();
            if (loc == nullptr) {
                loc = lastPage.appendAndReturnLocation (aggRec);

                // if we could not write, then the page was full
                if (loc == nullptr) {
                    MyDB_PageReaderWriter nextPage (true, *(input->getBufferMgr ()));
                    lastPage = nextPage;
                    allPages.push_back (lastPage);
                    loc = lastPage.appendAndReturnLocation (aggRec);
                }

                aggRec->fromBinary (loc);
                myHash [hashVal].push_back (loc);

                // otherwise, re-write to the old location
            }

            else {
                aggRec->toBinary (loc);
            }
        }
    }


//--------------------------------------------
    // output the result
    MyDB_RecordIteratorAltPtr myIterAgain = getIteratorAlt (allPages);


    MyDB_RecordPtr outRec = output->getEmptyRecord ();
    while (myIterAgain->advance ()) {

        myIterAgain->getCurrent (aggRec);


        i = 0;
        while(i < numGroups){
            outRec->getAtt (i)->set (aggRec->getAtt (i));
            i++;
        }


        for (auto &f : finalAggComps) {
            outRec->getAtt (i++)->set (f ());
        }
        outRec->recordContentHasChanged ();
        output->append (outRec);
    }

}

#endif

