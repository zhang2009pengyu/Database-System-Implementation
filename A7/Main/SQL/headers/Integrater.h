#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "MyDB_TableReaderWriter.h"
#include "MyDB_BPlusTreeReaderWriter.h"
#include "RelAlgExp.h"

using namespace std;

class Integrater {

public:
	Integrater (
		map <string, MyDB_TableReaderWriterPtr> &allTableReaderWriters,
		struct SFWQuery &mySFWQuery);

	MyDB_TableReaderWriterPtr integrate ();
private:
	map <string, MyDB_TableReaderWriterPtr> &allTableReaderWriters;

	vector <pair <string, string>> &tables;
	vector <ExprTreePtr> &disjunctions;
    vector <ExprTreePtr> &attributes;
    vector <ExprTreePtr> &groupings;

	pair <RelAlgExpTreePtr, int> handleMultiTable (vector < pair<string, string> > tables,
												   vector <ExprTreePtr> disjunctions,
												   vector <ExprTreePtr> attributes);

	pair <RelAlgExpTreePtr, int> handleOneTable (vector < pair<string, string> > tables,
												 vector <ExprTreePtr> disjunctions,
												 vector <ExprTreePtr> attributes);

	vector <ExprTreePtr> handleAtts(vector<pair<string, string>> tables,
									vector <ExprTreePtr> attsInTopCnf,
									vector <ExprTreePtr> attsToSelect);

};


#endif
