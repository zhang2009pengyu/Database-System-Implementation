#ifndef RELATIONAL_ALGEBRA_EXPRESSIONS
#define RELATIONAL_ALGEBRA_EXPRESSIONS

#include "Aggregate.h"
#include "ExprTree.h"
#include "MyDB_Schema.h"
#include "MyDB_Table.h"
#include "MyDB_TableReaderWriter.h"
#include "RegularSelection.h"
#include "ScanJoin.h"


using namespace std;

struct stats{
	size_t tupleCount;
	//attribute name and distinct value 
	vector <pair< string, size_t>> distinctValues;
};

class RelAlgExpTree;
typedef shared_ptr <RelAlgExpTree> RelAlgExpTreePtr;

class RelAlgExpTree {
public:
	virtual string toString () = 0;
	virtual ~RelAlgExpTree () {}
	virtual MyDB_TableReaderWriterPtr run () = 0;
	// static vector <int> availableIds;
	static int tempTableNum;
    // static int getId ();
    virtual string getSelectionPredicate() {
		string selectionPredicate = "bool[true]";
		for (auto expr : selectionDisjuncts) {
			selectionPredicate = "&& (" + expr->toString () + ", " + selectionPredicate + ")";
		}
		return selectionPredicate;
	}
    //virtual stats* getStats() =0;
    vector <ExprTreePtr> selectionDisjuncts;

protected:
	int id;
};


// SELECT *
// FROM table
// WHERE cnf
class SelectionOp : public RelAlgExpTree {
public:
	SelectionOp (vector <ExprTreePtr> disjuncts, RelAlgExpTreePtr inputIn, 
		      vector <ExprTreePtr> atts) {
		selectionDisjuncts = disjuncts;
		projectionAtts = atts;
		input = inputIn;
		id = -1;
	}
	
	string toString ();

	MyDB_TableReaderWriterPtr run ();
private:
	vector <ExprTreePtr> projectionAtts;
	RelAlgExpTreePtr input;

	
};


class TableLeaf : public RelAlgExpTree {
public:
	TableLeaf (MyDB_TableReaderWriterPtr tableIn, string tableShortNameIn) {
		table = tableIn;
		tableShortName = tableShortNameIn;
	}

	string toString ();

	stats* getStats();

	MyDB_TableReaderWriterPtr run ();


private:
	MyDB_TableReaderWriterPtr table;
	string tableShortName;
};

class JoinOp : public RelAlgExpTree{
public:
    JoinOp (vector <ExprTreePtr> disjuncts,
			RelAlgExpTreePtr leftIn,
			RelAlgExpTreePtr rightIn,
			vector <ExprTreePtr> atts);
    string toString();

	size_t getTopCNFSize ();
	MyDB_TableReaderWriterPtr  run ();
	
private:
	RelAlgExpTreePtr left;
	RelAlgExpTreePtr right;
	vector <ExprTreePtr> projectionAtts;
};

class AggregateOp : public RelAlgExpTree {
public:
	AggregateOp (RelAlgExpTreePtr inputIn,
				 vector <pair <pair <MyDB_AggType, string>,
						 MyDB_AttTypePtr>> aggsToComputeIn,
				 vector <ExprTreePtr> groupingsIn);

	string toString ();
	MyDB_TableReaderWriterPtr run ();
	
private:
	RelAlgExpTreePtr input;
	vector <pair <pair <MyDB_AggType, string>, MyDB_AttTypePtr>> aggsToCompute;
	vector <ExprTreePtr> groupings;

	string getSelectionPredicate ();
};

#endif
