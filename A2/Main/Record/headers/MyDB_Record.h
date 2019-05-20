
#ifndef RECORD_H
#define RECORD_H

#include <functional>
#include "MyDB_AttVal.h"
#include "MyDB_Schema.h"
#include <memory>
#include <string>
#include <vector>

using namespace std;

// create a smart pointer for records
class MyDB_Record;
typedef shared_ptr <MyDB_Record> MyDB_RecordPtr;

// a lambda function over the record... computes an attribute value
typedef function <MyDB_AttValPtr ()> func;

// a container for a database record... the way that this is typically used is that
// a MyDB_Record is created for a particualr table.  Then the record is used along
// with one of the ReaderWriter classes to access or to change the contents of the
// talbe

class MyDB_Record {

public:

	// constructs a record that can hold data for the given schema
	MyDB_Record (MyDB_SchemaPtr mySchema);

	// read the record from the text string
	void fromText (string fromMe);

	// get the number of bytes required to store the record as a binary string
	size_t getBinarySize ();

	// write the contents of the record in binary to the specifed location; return
	// the location of the next byte that was not writen... thus, the code:
	//
	// void *loc = startLoc;
	// for (int i = 0; i < 10; i++) {
	// 	loc = myRec.toBinary (loc);
	// }
	// 	
	// would write 10 copies of the record to the location pointed to by startLoc
	void *toBinary (void *toHere);

	// reverse of the above; the following code would read the next 10 records
	//
	// void *loc = startLoc;
	// for (int i = 0; i < 10; i++) {
	// 	loc = myRec.fromBinary (loc);
	// }
	// 	
	void *fromBinary (void *startPos);

	// parse the contents of this record from the given string
	void fromString (string fromMe);

	// write the record to an output string
	friend std::ostream& operator<<(std::ostream& os, const MyDB_Record printMe);
	friend std::ostream& operator<<(std::ostream& os, const MyDB_RecordPtr printMe);
	
	// destructor
	~MyDB_Record ();

	// builds a function over this record, as specified in the given string...
	// the string is an infix notation specification of a computation over
	// the record.  For example:
	//
	// myRecord.compileComputation ("+ ( [firstAtt], / ([secAtt], [secAtt])) ");
	// 
	// returns a function equivalent to the SQL: firstAtt + (secAtt / secAtt)
	//
	// myRecord.compileComputation ("< (+ (double [34.45], + ( [firstAtt], / ([secAtt], [secAtt]))), int [20]) ");
	//
	// returns a function equivalent to the SQL: (34.45 + firstAtt + (secAtt / secAtt)) < 20
	//
	// myRecord.compileComputation ("!= ( string [here is one], string[here is another] ) ");
	//
	// returns a function equivalent to the SQL: 'here is one' <> 'here is another'
	//
	// note that when the resulting lambda expression is called, it is called
	// by computing the function over the CURRENT version of the record.  So one
	// can create the function over the record, then load the record, and invoke
	// the lambda---this will evaluate the function over the current contents of the
	// record.  Then, if one loads different contents into the record and executes
	// the lambda again, the function can return a different value the second time
	// around.  This is useful, because one can use a single record to iterate through
	// the entire file, computing the function after each new record is loaded, without
	// recompiling the function.
	//
	func compileComputation (string fromMe);

private:
	// helper function for the compilation
	pair <func, MyDB_AttTypePtr> compileHelper (char * &vals);

	// helper function for the compilation
	char *findsymbol (char val, char *input);
	
	// these functions are all used to build up computations over the record
	pair <func, MyDB_AttTypePtr> fromData (string attName);
	pair <func, MyDB_AttTypePtr> plus (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs);
	pair <func, MyDB_AttTypePtr> minus (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs);
	pair <func, MyDB_AttTypePtr> times (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs);
	pair <func, MyDB_AttTypePtr> divide (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs);
	pair <func, MyDB_AttTypePtr> gt (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs);
	pair <func, MyDB_AttTypePtr> lt (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs);
	pair <func, MyDB_AttTypePtr> eq (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs);
	pair <func, MyDB_AttTypePtr> neq (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs);
	pair <func, MyDB_AttTypePtr> andd (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs);
	pair <func, MyDB_AttTypePtr> orr (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs);
	pair <func, MyDB_AttTypePtr> unaryMinus (pair <func, MyDB_AttTypePtr> lhs);
	pair <func, MyDB_AttTypePtr> nott (pair <func, MyDB_AttTypePtr> lhs);

private:

	// the schema for the record
	MyDB_SchemaPtr mySchema;

	// the actual values in this database record
	vector <MyDB_AttValPtr> values;	

	// stores pointers to all of the intermediate values created during function evaluation
	vector <MyDB_AttValPtr> scratch;

};

#endif
