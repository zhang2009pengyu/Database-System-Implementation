
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include "MyDB_Catalog.h"
#include <iostream>
#include <string>
#include <vector>

// create a smart pointer for database tables
using namespace std;

class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")


class ExprTree : public enable_shared_from_this <ExprTree> {

public:
	virtual string toString () = 0;
	virtual bool ini(MyDB_CatalogPtr catalog,
					   vector <pair <string, string>> tablesToProcess,
					   vector <ExprTreePtr> groups){
		return true;
	}

    MyDB_AttTypePtr getAttType () {
		return attType;
	}

	void setAttType (MyDB_AttTypePtr attTypeIn) {
		attType = attTypeIn;
	}

	string getAttName(){
		return attName;
	}
		
	virtual ~ExprTree () {}

	virtual bool isEquality () {
		return false;
	}

	virtual bool isIdentifier ()  {
		return false;
	}

	virtual bool isAgg () {
		return false;
	}

	virtual bool inTable (string table) {
		return false;
	};

	virtual void getAtts (vector <ExprTreePtr> &atts) {
		
	};

	virtual pair <string, string> getAgg () {
		return make_pair ("", "");
	}

	bool isIntFlag = false;
	bool isStringFlag = false;
	
protected:
	MyDB_AttTypePtr attType;
	// bool isIntFlag =false;
	// bool isStringFlag;
	string attName;
};

class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
		attType=make_shared <MyDB_BoolAttType> ();
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		}
		else {
			return "bool[false]";
		}
	}
	

	MyDB_AttTypePtr getAttType () {
		return attType;
	}
};

class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
		attType = make_shared <MyDB_DoubleAttType> ();

	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}


	~DoubleLiteral () {}
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
		attType = make_shared <MyDB_IntAttType> ();
		isIntFlag =true;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

	~IntLiteral () {}
};

class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
		attType = make_shared <MyDB_StringAttType> ();

	}

	string toString () {
		return "string[" + myVal + "]";
	}

	bool isString(){
		return true;
	}
	
	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
	string attTypeStr;
public:

	Identifier (const char *tableNameIn, const char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
		attTypeStr = "";
		// attName.push_back(tableName + "_" + tmp);
	}

	string toString () {
		if (tableName == "") {
			return "[" + attName + "]";
		}
		return "[" + tableName + "_" + attName+ "]";
	}

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		bool inCatalog = false;
		string tableFullName;
		for (auto table : tablesToProcess) {
			if (tableName == table.second) {
				tableFullName = table.first;
				inCatalog = true;
			}
		}
		vector <string> atts;
		catalog->getStringList(tableFullName + ".attList", atts);


		catalog->getString(tableFullName + "." + attName + ".type", attTypeStr);
        isIntFlag = (attTypeStr == "int");
        if (attTypeStr == "int") {
			attType = make_shared <MyDB_IntAttType> ();
		}
		else if (attTypeStr == "double") {
			attType =  make_shared <MyDB_DoubleAttType> ();
		}
		else if (attTypeStr == "string") {
			attType =  make_shared <MyDB_StringAttType> ();
		}
		else if (attTypeStr == "bool") {
			attType =  make_shared <MyDB_BoolAttType> ();
		}

		cout << "attType" << attType << endl;

		
		return true;
		
	}


	~Identifier () {}

	bool inTable (string table) {
		// cout << tableName << endl;
		if (table == tableName) {
			return true;
		}
		return false;
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		atts.push_back (shared_from_this ());
	}

	bool isIdentifier ()  {
		return true;
	}

	string getAttName ()  {

		return  tableName + "_" + attName;
	}

};

//arithmetic
class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		isIntFlag = lhs->isIntFlag && rhs->isIntFlag;
		if (isIntFlag) {
			attType = make_shared <MyDB_IntAttType> ();
		} else {
			attType = make_shared <MyDB_DoubleAttType> ();
		}
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}



	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!lhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		if (!rhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		return true;
	}

	~MinusOp () {}
	
	bool inTable (string table) {
		return lhs->inTable (table) || rhs->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		lhs->getAtts (atts);
		rhs->getAtts (atts);
	}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		isIntFlag = lhs->isIntFlag && rhs->isIntFlag;
		isStringFlag = lhs->isStringFlag && rhs->isStringFlag;
		if (isIntFlag) {
			attType = make_shared <MyDB_IntAttType> ();
		} 
		else if(isStringFlag){
			attType = make_shared <MyDB_StringAttType> ();
		}
		else{
			attType = make_shared <MyDB_DoubleAttType> ();
		}
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!lhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		if (!rhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		return true;
	}


	~PlusOp () {}

	bool inTable (string table) {
		return lhs->inTable (table) || rhs->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		lhs->getAtts (atts);
		rhs->getAtts (atts);
	}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		isIntFlag = lhs->isIntFlag && rhs->isIntFlag;
		if (isIntFlag) {
			attType = make_shared <MyDB_IntAttType> ();
		} else {
			attType = make_shared <MyDB_DoubleAttType> ();
		}

	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!lhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		if (!rhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		return true;
	}

	~TimesOp () {}
	bool inTable (string table) {
		return lhs->inTable (table) || rhs->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		lhs->getAtts (atts);
		rhs->getAtts (atts);
	}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		attType = make_shared <MyDB_DoubleAttType> ();


	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!lhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		if (!rhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		return true;
	}


	~DivideOp () {}

	bool inTable (string table) {
		return lhs->inTable (table) || rhs->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		lhs->getAtts (atts);
		rhs->getAtts (atts);
	}
};

//compare
class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		attType = make_shared <MyDB_BoolAttType> ();

	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!lhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		if (!rhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		return true;
	}

	~GtOp () {}

	bool inTable (string table) {
		return lhs->inTable (table) || rhs->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		lhs->getAtts (atts);
		rhs->getAtts (atts);
	}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		attType = make_shared <MyDB_BoolAttType> ();
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!lhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		if (!rhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		return true;
	}
	~LtOp () {}

	bool inTable (string table) {
		return lhs->inTable (table) || rhs->inTable (table);
	}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;

public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		attType = make_shared <MyDB_BoolAttType> ();
	}

	ExprTreePtr getlhs () {
		return lhs;
	}

	ExprTreePtr getrhs () {
		return rhs;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool ini(MyDB_CatalogPtr catalog,
			 vector <pair <string, string>> tablesToProcess,
			 vector <ExprTreePtr> groups) {
		if (!lhs->ini(catalog, tablesToProcess,
					  groups)) {
			return false;
		}
		if (!rhs->ini(catalog, tablesToProcess,
					  groups)) {
			return false;
		}
		return true;
	}

	~EqOp () {}

	bool isEquality () {
		return true;
	}

	bool inTable (string table) {
		// cout << "==" << endl;
		return lhs->inTable (table) || rhs->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		lhs->getAtts (atts);
		rhs->getAtts (atts);
	}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		attType = make_shared <MyDB_BoolAttType> ();
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!lhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		if (!rhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		return true;
	}

	~NeqOp () {}

	bool inTable (string table) {
		return lhs->inTable (table) || rhs->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		lhs->getAtts (atts);
		rhs->getAtts (atts);
	}
};


//logical
class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
		attType = make_shared <MyDB_BoolAttType> ();
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!lhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		if (!rhs->ini(catalog, tablesToProcess,
						groups)) {
			return false;
		}
		
		return true;
	}

	~OrOp () {}

	bool inTable (string table) {
		return lhs->inTable (table) || rhs->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		lhs->getAtts (atts);
		rhs->getAtts (atts);
	}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
		attType = make_shared <MyDB_BoolAttType> ();
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}	

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!child->ini(catalog, tablesToProcess,
				groups)) {
			return false;
		}
		return true;
	}
	
	~NotOp () {}

	bool inTable (string table) {
		return child->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		child->getAtts (atts);
	}
};

//function
class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
		attType = child->getAttType ();
				isIntFlag = child->isIntFlag;

	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!child->ini(catalog, tablesToProcess,
						  groups)) {
			return false;
		}
		return true;
	}

	~SumOp () {}

	bool inTable (string table) {
		return child->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		child->getAtts (atts);
	}

	bool isAgg () {
		return true;
	}

	pair <string, string> getAgg () {
		return make_pair ("sum", child->toString ());
	}

};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
		attType = make_shared <MyDB_DoubleAttType> ();

	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}

	bool ini(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups) {
		if (!child->ini(catalog, tablesToProcess, groups)) {
			return false;
		}
		return true;
	}

	~AvgOp () {}

	bool inTable (string table) {
		return child->inTable (table);
	}

	void getAtts (vector <ExprTreePtr> &atts) {
		child->getAtts (atts);
	}

	bool isAgg () {
		return true;
	}
	pair <string, string> getAgg () {
		return make_pair ("avg", child->toString ());
	}
};

#endif
