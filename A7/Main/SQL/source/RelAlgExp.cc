#ifndef RELATIONAL_ALGEBRA_EXPRESSIONS_CC
#define RELATIONAL_ALGEBRA_EXPRESSIONS_CC

#include "RelAlgExp.h"
#include <vector>

using namespace std;

int RelAlgExpTree::tempTableNum = 0;

//SelectionOp implements
MyDB_TableReaderWriterPtr SelectionOp::run () {
	MyDB_TableReaderWriterPtr inputTable = input->run ();
	id = tempTableNum++;
	string tableOutName = "temp" + to_string (id);

	MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();
	int i = 0;
	for(auto p: projectionAtts){
		string attName;
		if (p->isIdentifier ()){
			attName = static_pointer_cast <Identifier> (p)->getAttName ();
		}
		else{
			attName = tableOutName + "_Att_" + to_string (i++);
		}
		mySchemaOut->appendAtt(make_pair (attName,p->getAttType()));
	}

	MyDB_TablePtr myTableOut = make_shared <MyDB_Table> (tableOutName, tableOutName + ".bin",mySchemaOut);
	cout << "myTableOutName, " << tableOutName << endl;
	MyDB_TableReaderWriterPtr outputTable = make_shared <MyDB_TableReaderWriter>
		(myTableOut, inputTable->getBufferMgr ());

	vector <string> projections;
	for (auto p: projectionAtts){
		projections.push_back(p->toString());
	}
	MyDB_RecordPtr outputRec = outputTable->getEmptyRecord ();

	RegularSelection myOp (inputTable, outputTable, getSelectionPredicate (), projections);
	myOp.run ();

	return outputTable;
}

string SelectionOp::toString () {
	return "select (" + input->toString () + ") where " + getSelectionPredicate ();
}

//TableLeaf implements
string TableLeaf::toString () {
	return "table[" + table->getTable ()->getName () + "]" ;
}

stats* TableLeaf::getStats(){
    stats* myStats;
    myStats->tupleCount = table-> getTable() -> getTupleCount();
    //vector <pair <string, MyDB_AttTypePtr>> &getAtts ()
    for(auto a: table-> getTable() ->getSchema() -> getAtts()){
        myStats->distinctValues.push_back(make_pair(a.first, table-> getTable() -> getDistinctValues(a.first)));
    }
    return myStats;
}

MyDB_TableReaderWriterPtr TableLeaf::run () {
	MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();
	for (auto p : table->getTable ()->getSchema ()->getAtts ()) {

		if (tableShortName != "") {
			mySchemaOut->appendAtt (
					make_pair(tableShortName + "_" + p.first,
							  p.second));
		}
		else {
			mySchemaOut->appendAtt (
					make_pair(p.first,
							  p.second));
		}
	}
	id = tempTableNum++;
	string tableOutName = "temp" + to_string (id);

	MyDB_TablePtr myTableOut = make_shared <MyDB_Table> (
			tableOutName, tableOutName + ".bin",
			mySchemaOut);
	MyDB_TableReaderWriterPtr outputTable = make_shared <MyDB_TableReaderWriter>
			(myTableOut, table->getBufferMgr ());

	vector <string> projections;
	for (auto p: table->getTable ()->getSchema ()->getAtts ()){
		//not limit to attribute
		projections.push_back("[" + p.first + "]");
	}

	RegularSelection myOp (table, outputTable,
						   "bool[true]", projections);

	myOp.run ();
	cout <<"test in outputRec" << endl;
	MyDB_RecordPtr outputRec = outputTable->getEmptyRecord ();
	return outputTable;
}



//JoinOp implements
JoinOp::JoinOp (vector <ExprTreePtr> disjuncts,
				RelAlgExpTreePtr leftIn,
				RelAlgExpTreePtr rightIn,
				vector <ExprTreePtr> atts):
		left(leftIn),right(rightIn){
			selectionDisjuncts = disjuncts;
			projectionAtts = atts;
		}

string JoinOp::toString(){
	return "join (" + left->toString () + ", " + 
		right->toString () + ")";
}


size_t JoinOp::getTopCNFSize () {
	return selectionDisjuncts.size ();
}

MyDB_TableReaderWriterPtr JoinOp::run () {
	MyDB_TableReaderWriterPtr leftInput = left->run();
	MyDB_TableReaderWriterPtr rightInput = right->run();

    leftInput->writeIntoTextFile ("leftSelectionResult");
    rightInput->writeIntoTextFile ("rightSelectionResult");

	id = tempTableNum++;
	string tableOutName = "temp" + to_string (id);

	MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();

	int i = 0;
	for(auto p: projectionAtts){
		string attName;
		if (p->isIdentifier ()) {
			attName = static_pointer_cast <Identifier> (p)->getAttName ();
		}
		else {
			attName = tableOutName + "_Att_" + to_string (i++);
		}
		auto attType = leftInput->getTable()->getSchema()->getAttByName(attName).second;
		if(attType == nullptr){
			attType = rightInput->getTable()->getSchema()->getAttByName(attName).second;
		}
		mySchemaOut->appendAtt (make_pair (attName, p->getAttType()));
	}

	MyDB_TablePtr myTableOut = make_shared <MyDB_Table>( tableOutName, tableOutName + ".bin", mySchemaOut);
	MyDB_TableReaderWriterPtr outputTable = make_shared <MyDB_TableReaderWriter>
		(myTableOut, leftInput->getBufferMgr ());

	vector <string> projections;
	for (auto p: projectionAtts){
		projections.push_back(p->toString());
	}

	vector <pair<string,string>> equalitychecks;
		
	for (auto cnf: selectionDisjuncts) {
		if (cnf->isEquality ()){
			pair<string, string> equalityString;
			ExprTreePtr lhs = static_pointer_cast<EqOp>(cnf)->getlhs();
			bool isLeft = false;

			for (auto p : leftInput->getTable ()->getSchema ()->getAtts ()) {
				if (lhs->toString () == "[" + p.first + "]") {
					isLeft = true;
					break;
				}
			}
			if (isLeft) {
				equalityString.first = lhs->toString();
				equalityString.second = static_pointer_cast<EqOp>(cnf)->getrhs()->toString();
			}

			else {
				equalityString.second = lhs->toString();
				equalityString.first = static_pointer_cast<EqOp>(cnf)->getrhs()->toString();
			}
			
			
			equalitychecks.push_back(equalityString);
		}
	}
	ScanJoin myOp(leftInput, rightInput, outputTable, getSelectionPredicate(),
        	           projections, equalitychecks, "bool[true]", "bool[true]" );
	myOp.run();

	return outputTable;
}


//AggregateOp

AggregateOp::AggregateOp (RelAlgExpTreePtr inputIn,
						  vector <pair <pair <MyDB_AggType, string>,
								  MyDB_AttTypePtr>> aggsToComputeIn,
						  vector <ExprTreePtr> groupingsIn):
		aggsToCompute (aggsToComputeIn), groupings (groupingsIn) {
	input = inputIn;
}

string AggregateOp::toString () {
    return "Aggregate (" + input->toString () + ") selecting " +
           getSelectionPredicate ();
}

string AggregateOp::getSelectionPredicate () {
    string selectionStr = "";
    for (auto a : groupings) {
        selectionStr += a->toString () + "|";
    }
    for (auto p : aggsToCompute) {
        selectionStr += p.first.second + "| ";
    }
    return selectionStr;
}


MyDB_TableReaderWriterPtr AggregateOp::run () {
	
	MyDB_TableReaderWriterPtr inputTable = input->run ();

	id = tempTableNum++;
	string tableOutName = "temp" + to_string (id);

	MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();
	int i = 0;

	for(auto p: groupings){
		string attName;
		if (p->isIdentifier ()) {
			attName = static_pointer_cast <Identifier> (p)->getAttName ();
		}
		else {
			attName = tableOutName + "_Att_" + to_string (i++);
		}
		mySchemaOut->appendAtt (make_pair (attName,p->getAttType()));
	}

	for (auto p : aggsToCompute) {
	    mySchemaOut->appendAtt (make_pair (tableOutName + "_Att_" + to_string (i++),p.second));
	}

	MyDB_TablePtr myTableOut = make_shared <MyDB_Table> (
		tableOutName, tableOutName + ".bin",mySchemaOut
		);
	MyDB_TableReaderWriterPtr outputTable = make_shared <MyDB_TableReaderWriter>
		(myTableOut, inputTable->getBufferMgr ());

	vector <string> groupingStrs;
	for( auto exp : groupings){
		groupingStrs.push_back (exp->toString ());
	}

	vector<pair<MyDB_AggType, string>> newAggsToCompute;
	for (auto p : aggsToCompute) {
		newAggsToCompute.push_back (p.first);
	}
	Aggregate myOp (inputTable, outputTable, newAggsToCompute, groupingStrs, "bool[true]");
	myOp.run ();
	
	return outputTable;
}

#endif
