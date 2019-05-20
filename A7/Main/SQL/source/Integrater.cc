#ifndef OPTIMIZER_CC
#define OPTIMIZER_CC


#include "ExprTree.h"
#include "RelAlgExp.h"
#include "Integrater.h"
#include "ParserTypes.h"
#include <stack>
#include <bitset>
#include <cmath>

using namespace std;

Integrater::Integrater (
	map <string, MyDB_TableReaderWriterPtr> &allTableReaderWritersIn,
	struct SFWQuery &mySFWQuery):
	allTableReaderWriters( allTableReaderWritersIn),
	tables( mySFWQuery.getTables()),
	disjunctions( mySFWQuery.getAllDisjuncts()),
	attributes( mySFWQuery.getAllAtts()),
	groupings( mySFWQuery.getAllGroupings()) {
}


MyDB_TableReaderWriterPtr Integrater::integrate() {
    vector< ExprTreePtr> newAtts;
    vector <pair <pair <MyDB_AggType, string>, MyDB_AttTypePtr>> aggsToCompute;
    for( auto a : attributes){
    	a->getAtts( newAtts);
    	if (a->isAgg()){
    		pair< string, string> agg = a->getAgg ();
    		if( agg.first == "sum") {
    			aggsToCompute.push_back( make_pair( make_pair (MyDB_AggType :: sumAgg, agg.second), a->getAttType ()));
    		} 
    		else{
    			aggsToCompute.push_back( make_pair( make_pair (MyDB_AggType :: avgAgg, agg.second), a->getAttType ()));
    		} 

	
    	} 
    }

    pair<RelAlgExpTreePtr, int> optRet;
    if(tables.size() == 1)
    	optRet = handleOneTable(tables, disjunctions, newAtts);
    else
    	optRet = handleMultiTable(tables, disjunctions, newAtts);

    RelAlgExpTreePtr plan = optRet.first;

    if (!aggsToCompute.empty ()) {
    	plan = make_shared <AggregateOp> (plan, aggsToCompute, groupings);
    }

    cout << "The plan is (" << optRet.second << "): " << plan->toString () << endl;

    auto begin = clock();
    MyDB_TableReaderWriterPtr output = plan->run ();

    vector< pair< string, MyDB_AttTypePtr>> outputAtts = output->getTable ()->getSchema ()->getAtts ();
    int i = groupings.size ();
    vector <ExprTreePtr> attsToSelect;
    for (auto a : attributes) {
    	if (a->isAgg ()) {
    		string tableName = "";
    		ExprTreePtr aggAtt = make_shared <Identifier> (tableName.c_str (), outputAtts[i].first.c_str ());
    		aggAtt->setAttType (outputAtts[i].second);
    		attsToSelect.push_back (aggAtt);
    		i++;
    	} 
    	else {
    		attsToSelect.push_back (a);
    	}
    }

    plan = make_shared<SelectionOp> (vector <ExprTreePtr> (), make_shared <TableLeaf> (output, ""), attsToSelect);

    output = plan->run ();

    auto time = clock()-begin;
    float runningtime = (float)time / CLOCKS_PER_SEC;
	cout << "Query run time: " << runningtime <<"seconds "<<endl;
    
    return output;
}

pair <RelAlgExpTreePtr, int> Integrater::handleOneTable (vector < pair<string, string> > tables,
														 vector <ExprTreePtr> disjunctions,
	vector <ExprTreePtr> attributes){
		string tableName = tables[0].first;
		RelAlgExpTreePtr table = make_shared <TableLeaf> (allTableReaderWriters[tableName], tables[0].second);
		RelAlgExpTreePtr selectionOp = make_shared <SelectionOp> (disjunctions, table, attributes);
		return make_pair (selectionOp, 0);
}

pair <RelAlgExpTreePtr, int> Integrater::handleMultiTable (vector < pair<string, string> > tables,
														   vector <ExprTreePtr> disjunctions,
	vector <ExprTreePtr> attributes) {
	    int subsetNo = pow(2, tables.size());
	    int bitsetNo = subsetNo / 2;
		vector<bitset<128>> binarylist;
		for(int i = 1; i < bitsetNo; i++){
			int index = 0;
			int value = i;
			bitset<128> bit;
	        while(value){
	          if(value % 2){
	          	bit.set(index);
	          }
	          value = value / 2;
	          index++;
	       }
	       binarylist.push_back(bit); 
		}
		vector< pair< vector< pair<string, string> >, vector< pair<string, string> > > > allSubsetPair;
	    for(auto b: binarylist){
	    	pair < vector < pair < string, string >>, vector < pair < string, string >>> subset;
	    	for(int index = 0; index < tables.size(); index++){
	    		if(b[index]){
	    			subset.first.push_back(tables[index]);
	    		}
	    		else{
	    			subset.second.push_back(tables[index]);
	    		}
	    	}
	    	allSubsetPair.push_back(subset);
	    }

        int maxCNFNum = -1;
        RelAlgExpTreePtr bestPlan = nullptr;

	    for (auto p : allSubsetPair) {    

	        vector<pair<string, string>> lefttables = p.first;
	        vector<pair<string, string>> righttables = p.second;
        	cout << endl;

	        vector <ExprTreePtr> leftcnf;
	        vector <ExprTreePtr> rightcnf;
	        vector <ExprTreePtr> topcnf;

	        for(auto c : disjunctions){
				bool inleft = false;
				bool inright = false;
				for(auto it: lefttables){
					if(c->inTable(it.second)){
						inleft = true;
						break;
					}
				}
	            for(auto it:righttables){
	            	if(c->inTable(it.second)){
	            		inright = true;
	            		break;
	            	}
	            }
	            

				if ( inleft && inright)
	            	topcnf.push_back(c);
	            else if (inleft)
	            	leftcnf.push_back(c);
				else if ( inright)
	            	rightcnf.push_back(c);
	        }

	        vector <ExprTreePtr> attsInTopCnf;
	        for(auto topc: topcnf)
				topc->getAtts (attsInTopCnf);
	        vector <ExprTreePtr> attsToSelect;
	        for (auto att : attributes)
	        	att->getAtts (attsToSelect);

			auto leftAtts = handleAtts(lefttables, attsInTopCnf, attsToSelect);
			auto rightAtts= handleAtts(righttables,attsInTopCnf, attsToSelect);

	        auto leftRet = handleMultiTable(lefttables, leftcnf, leftAtts);
	        auto rightRet = handleMultiTable(righttables, rightcnf, rightAtts);

	       	int currCost = leftRet.second + rightRet.second;
	        if (currCost > maxCNFNum) {
	        	maxCNFNum = currCost;
	        	bestPlan = make_shared <JoinOp> (topcnf, leftRet.first, rightRet.first, attributes);
	        	cout << "Find a better plan ("	<<  maxCNFNum << "): " << bestPlan->toString () << endl;

	        }
	    }    
		return make_pair (bestPlan, maxCNFNum + static_pointer_cast <JoinOp> (bestPlan)->getTopCNFSize ());
}

vector <ExprTreePtr> Integrater::handleAtts(vector<pair<string, string>> tables,
											vector <ExprTreePtr> attsInTopCnf,
											vector <ExprTreePtr> attsToSelect){
	vector<ExprTreePtr> returnAtts;
	vector<pair<string, MyDB_AttTypePtr>> attsPart;
	for(auto it: tables){
        auto attsInEvery = allTableReaderWriters[it.first]->getTable()->getSchema ()->getAtts ();
        for (auto p : attsInEvery) {
			attsPart.push_back (make_pair (it.second + "_" + p.first, p.second));
		}
	}
	
    for (auto p : attsPart) {
    	for (auto a : attsInTopCnf) {
			if (a->toString () == ("[" + p.first + "]")) {
				returnAtts.push_back (a);
			}
		}
		for (auto a : attsToSelect) {
			if (a->toString () == ("[" + p.first + "]")) {
				returnAtts.push_back (a);
			}
		}
	}
	return returnAtts;
}


#endif
