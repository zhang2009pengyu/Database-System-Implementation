
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include "MyDB_Catalog.h"
#include <string>
#include <cstring>
#include <vector>
#include <set>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;

//add this enum type
enum  Expr_Type {NumberType, BoolType, StringType, NoneType};

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:
	virtual string toString () = 0;

	virtual bool check(MyDB_CatalogPtr catalog,
						vector<pair<string, string>> tablesToProcess,
						vector<ExprTreePtr> groups,
						bool checkAvg) = 0;

	virtual Expr_Type getType(MyDB_CatalogPtr catalog,
							  vector<pair<string, string>> tablesToProcess) = 0;

	string typeToString (Expr_Type exprType){
		switch (exprType){
			case NumberType:
				return "number (int or double)";
			case BoolType:
				return "bool";
			case StringType:
				return "string";
			case NoneType:
				return "none";
		}
	}


	virtual ~ExprTree () {}
};

//bool, double, int, string
//same processing method
//simple condition
class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}


    //implement
    bool check(MyDB_CatalogPtr catalog,
               vector <pair <string, string>> tablesToProcess,
               vector <ExprTreePtr> groups, bool checkAgg) {
        return true;
    }
    //implement
    Expr_Type getType(MyDB_CatalogPtr catalog,
                      vector <pair <string, string>> tablesToProcess) {
        return BoolType;
    }
};

//simple condition
class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}


	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {
		return true;
	}
	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {
		return NumberType;
	}


	~DoubleLiteral () {}
};

//simple condition
class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {
		return true;
	}
	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {
		return NumberType;
	}

	~IntLiteral () {}
};

//simple condition
class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
	}

	string toString () {
		return "string[" + myVal + "]";
	}

	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {
		return true;
	}
	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {
		return StringType;
	}

	~StringLiteral () {}
};

//---------------------
//little complex ??
class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
	string attType;
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
		attType = "";
	}

	string toString () {
		return "[" + tableName + "_" + attName + "]";
	}


	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {

		bool inCatalog = false;
		string tableCompleteName;
		//find the completename of the attribute's table
		for (auto table : tablesToProcess) {
			if (tableName == table.second) {
				tableCompleteName = table.first;
				inCatalog = true;
			}
		}


		if (!inCatalog) {
			cout << "Error: Unknown table name: " << tableName << endl;
			return false;
		}
//        check table names finished.

		vector <string> atts;
		catalog->getStringList(tableCompleteName + ".attList", atts);

		set<string> attsSet(atts.begin(), atts.end());
		bool inTable = false;
		if(attsSet.find(attName) != attsSet.end()){
			inTable = true;
		}


		if (!inTable) {
			cout << "Error: Unknown attribute name: " << attName << endl;
			return false;
		}
//      check attribute names finished.


		catalog->getString(tableCompleteName + "." + attName + ".type", attType);


		if (checkAgg) {
			bool inGroup = false;

			for (ExprTreePtr group : groups) {
				if (toString() == group->toString()) {
					inGroup = true;
				}
			}
			if (!inGroup) {
				cout << "Error: Selected attribute: " << toString() <<
					 " is not in the grouping attributes" << endl;
				return false;
			}
		}
		return true;
	}
	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {
		// make sure call check before getType
		if (attType == "bool") {
			return BoolType;
		}


		if ((attType == "int") || (attType == "double")) {
			return NumberType;
		}


		if (attType == "string") {
			return StringType;
		}

		return NoneType;
	}


	~Identifier () {}
};


//----------------------------
//arithmetic operators: minus, plus, times, divide
class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}


	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {

        //the operand must satisfy the requirement of sql
		if (!lhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}
		if (!rhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}

        //the type of operand can only be NumberType
		if (lhs->getType(catalog, tablesToProcess) != NumberType) {
			cout << "Error: The lhs of operator -: " << lhs->toString() << " is of type "
				 << ExprTree::typeToString(lhs->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of - be number." << endl;
			return false;
		}
		if (rhs->getType(catalog, tablesToProcess) != NumberType) {
			cout << "Error: The rhs of operator -: " << rhs->toString() << " is of type "
				 << ExprTree::typeToString(rhs->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of - be number." << endl;
			return false;
		}

		return true;
	}
	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {
        //the result of minus is NumberType
		if ((lhs->getType(catalog, tablesToProcess) == NumberType)
			&& (rhs->getType(catalog, tablesToProcess) == NumberType)) {
			return NumberType;
		}
		return NoneType;
	}

	~MinusOp () {}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}


	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {

        //the operand must satisfy the requirement of sql
		if (!lhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}
		if (!rhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}

        //the type of operand can only be NumberType or StringType
		if ((lhs->getType(catalog, tablesToProcess) == NumberType)
			&& (rhs->getType(catalog, tablesToProcess) == NumberType)) {
			return true;
		}

		if ((lhs->getType(catalog, tablesToProcess) == StringType)
			&& (rhs->getType(catalog, tablesToProcess) == StringType)) {
			return true;
		}
		cout << "Error: Operator + applied to " << lhs->toString() << " and  "
			 << rhs->toString() << " we only allows type number and type string." << endl;

		return false;
	}

	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {

        //the result of times is NumberType or StringType
		if ((lhs->getType(catalog, tablesToProcess) == NumberType)
			&& (rhs->getType(catalog, tablesToProcess) == NumberType)) {
			return NumberType;
		}
		if ((lhs->getType(catalog, tablesToProcess) == StringType)
			&& (rhs->getType(catalog, tablesToProcess) == StringType)) {
			return StringType;
		}
		return NoneType;
	}

	~PlusOp () {}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {

        //the operand must satisfy the requirement of sql
		if (!lhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}
		if (!rhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}

		if (lhs->getType(catalog, tablesToProcess) != NumberType) {
			cout << "Error: The lhs of operator *: " << lhs->toString() << " is of type "
				 << ExprTree::typeToString(lhs->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of * be number." << endl;
			return false;
		}
		if (rhs->getType(catalog, tablesToProcess) != NumberType) {
			cout << "Error: The rhs of operator *: " << rhs->toString() << " is of type "
				 << ExprTree::typeToString(rhs->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of - be number." << endl;
			return false;
		}

		return true;
	}
	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {
        //the result of times is NumberType
		if ((lhs->getType(catalog, tablesToProcess) == NumberType)
			&& (rhs->getType(catalog, tablesToProcess) == NumberType)) {
			return NumberType;
		}
		return NoneType;
	}

	~TimesOp () {}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {

        //the operand must satisfy the requirement of sql
		if (!lhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}
		if (!rhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}

		if (lhs->getType(catalog, tablesToProcess) != NumberType) {
			cout << "Error: The lhs of operator /: " << lhs->toString() << " is of type "
				 << ExprTree::typeToString(lhs->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of / be number." << endl;
			return false;
		}
		if (rhs->getType(catalog, tablesToProcess) != NumberType) {
			cout << "Error: The rhs of operator /: " << rhs->toString() << " is of type "
				 << ExprTree::typeToString(rhs->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of / be number." << endl;
			return false;
		}

		return true;
	}
	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {
        //the result of divide is NumberType
		if ((lhs->getType(catalog, tablesToProcess) == NumberType)
			&& (rhs->getType(catalog, tablesToProcess) == NumberType)) {
			return NumberType;
		}
		return NoneType;
	}


	~DivideOp () {}
};



//----------------------------
//relational operators: gt, lt, neq, eq，
//logical operators: or, not
class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {

        //the operand must satisfy the requirement of sql
		if (!lhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}
		if (!rhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}

        //the type of operand can only be NumberType or StringType
		if ((lhs->getType(catalog, tablesToProcess) == NumberType)
			&& (rhs->getType(catalog, tablesToProcess) == NumberType)) {
			return true;
		}

		if ((lhs->getType(catalog, tablesToProcess) == StringType)
			&& (rhs->getType(catalog, tablesToProcess) == StringType)) {
			return true;
		}
        cout << "Error: Operator > applied to " << lhs->toString() << " and  "
             << rhs->toString() << ". They must be in the same type of number or string." << endl;

		return false;
	}
	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {
        //the result of GT is BoolType
		if ((lhs->getType(catalog, tablesToProcess) == NumberType)
			&& (rhs->getType(catalog, tablesToProcess) == NumberType)) {
			return BoolType;
		}
		if ((lhs->getType(catalog, tablesToProcess) == StringType)
			&& (rhs->getType(catalog, tablesToProcess) == StringType)) {
			return BoolType;
		}
		return NoneType;
	}

	~GtOp () {}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {
        //the operand must satisfy the requirement of sql
		if (!lhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}
		if (!rhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}
        //the type of operand can only be NumberType or StringType
		if ((lhs->getType(catalog, tablesToProcess) == NumberType)
			&& (rhs->getType(catalog, tablesToProcess) == NumberType)) {
			return true;
		}

		if ((lhs->getType(catalog, tablesToProcess) == StringType)
			&& (rhs->getType(catalog, tablesToProcess) == StringType)) {
			return true;
		}
        cout << "Error: Operator > applied to " << lhs->toString() << " and  "
             << rhs->toString() << ". They must be in the same type of number or string." << endl;


        return false;
	}
	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {
        //the result of LT is BoolType
		if ((lhs->getType(catalog, tablesToProcess) == NumberType)
			&& (rhs->getType(catalog, tablesToProcess) == NumberType)) {
			return BoolType;
		}
		if ((lhs->getType(catalog, tablesToProcess) == StringType)
			&& (rhs->getType(catalog, tablesToProcess) == StringType)) {
			return BoolType;
		}
		return NoneType;
	}

	~LtOp () {}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}


	//implement
	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {
        //the operand must satisfy the requirement of sql
		if (!lhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}
		if (!rhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}

        //the type of operand must be the same
		if (lhs->getType(catalog, tablesToProcess)
			!= rhs->getType(catalog, tablesToProcess)) {
            cout << "Error: Operator != applied to " << lhs->toString() << " and  "
                 << rhs->toString() << " they must be in the same type" << endl;
			return false;
		}

		return true;
	}
	//implement
	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {

		if (lhs->getType(catalog, tablesToProcess)
			== rhs->getType(catalog, tablesToProcess)) {
			return BoolType;
		}
		return NoneType;
	}

	~NeqOp () {}
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {
		if (!lhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}
		if (!rhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}

		if (lhs->getType(catalog, tablesToProcess) != BoolType) {
			cout << "Error: The lhs of operator ||: " << lhs->toString() << " is of type "
				 << ExprTree::typeToString(lhs->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of || be booltype." << endl;
			return false;
		}
		if (rhs->getType(catalog, tablesToProcess) != BoolType) {
			cout << "Error: The rhs of operator ||: " << rhs->toString() << " is of type "
				 << ExprTree::typeToString(rhs->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of || be booltype." << endl;
			return false;
		}

		return true;
	}

	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {
        //the result of or is BoolType
		if ((lhs->getType(catalog, tablesToProcess) == BoolType)
			&& (rhs->getType(catalog, tablesToProcess) == BoolType)) {
			return BoolType;
		}
		return NoneType;
	}

	~OrOp () {}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}


	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {

        //the operand must satisfy the requirement of sql
		if (!lhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}
		if (!rhs->check(catalog, tablesToProcess,
						groups, checkAgg)) {
			return false;
		}

		if (lhs->getType(catalog, tablesToProcess)
			!= rhs->getType(catalog, tablesToProcess)) {
            cout << "Error: Operator == applied to " << lhs->toString() << " and  "
                 << rhs->toString() << " they must be in the same type" << endl;
			return false;
		}

		return true;
	}

	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {

		if (lhs->getType(catalog, tablesToProcess)
			== rhs->getType(catalog, tablesToProcess)) {
			return BoolType;
		}
		return NoneType;
	}

	~EqOp () {}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}

	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {
		if (!child->check(catalog, tablesToProcess,
						  groups, checkAgg)) {
			return false;
		}


		if (child->getType(catalog, tablesToProcess) != BoolType) {
			cout << "Error: The child of operator !: "
				 << child->toString() << " is of type "
				 << ExprTree::typeToString(child->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of not be bool." << endl;
			return false;
		}

		return true;
	}

	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {

		if ((child->getType(catalog, tablesToProcess) == BoolType)) {
			return BoolType;
		}
		return NoneType;
	}

	~NotOp () {}
};


//counting function: sum, avg
class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}

	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {
		if (!child->check(catalog, tablesToProcess,
						  groups, false)) {
			return false;
		}


		if (child->getType(catalog, tablesToProcess) != NumberType) {
			cout << "Error: The child of operator sum: "
				 << child->toString() << " is of type "
				 << ExprTree::typeToString(child->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of sum be number." << endl;
			return false;
		}

		return true;
	}

	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {

		if ((child->getType(catalog, tablesToProcess) == NumberType)) {
			return NumberType;
		}
		return NoneType;
	}


	~SumOp () {}
};


class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}


	bool check(MyDB_CatalogPtr catalog,
			   vector <pair <string, string>> tablesToProcess,
			   vector <ExprTreePtr> groups, bool checkAgg) {
		if (!child->check(catalog, tablesToProcess,
						  groups, false)) {
			return false;
		}


		if (child->getType(catalog, tablesToProcess) != NumberType) {
			cout << "Error: The child of operator avg: "
				 << child->toString() << " is of type "
				 << ExprTree::typeToString(child->getType(catalog, tablesToProcess))
                 << ". But we only allows the operand type of avg be number." << endl;
			return false;
		}

		return true;
	}

	Expr_Type getType(MyDB_CatalogPtr catalog,
					  vector <pair <string, string>> tablesToProcess) {

		if ((child->getType(catalog, tablesToProcess) == NumberType)) {
			return NumberType;
		}
		return NoneType;
	}

	~AvgOp () {}
};

#endif
