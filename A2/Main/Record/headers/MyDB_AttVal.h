
#ifndef ATT_VAL_H
#define ATT_VAL_H

#include <memory>
#include <string>

// create a smart pointer for the catalog
using namespace std;
class MyDB_AttVal;
typedef shared_ptr <MyDB_AttVal> MyDB_AttValPtr;

// this encapsulates actual data that is stored in the My_DB database system
class MyDB_AttVal {

public:

	virtual int toInt () = 0;
	virtual double toDouble () = 0;
	virtual string toString () = 0;
	virtual bool toBool () = 0;
	virtual void fromString (string &fromMe) = 0;
	virtual void *fromBinary (void *fromHere) = 0;
	virtual void *toBinary (void *toHere) = 0;
	virtual size_t getBinarySize () = 0;
	virtual ~MyDB_AttVal ();
	
};

class MyDB_IntAttVal;
typedef shared_ptr <MyDB_IntAttVal> MyDB_IntAttValPtr;

// an integer attribute
class MyDB_IntAttVal : public MyDB_AttVal {

public:

	int toInt () override;
	double toDouble () override;
	string toString () override;
	bool toBool () override;
	void fromString (string &fromMe) override;
	void *fromBinary (void *fromHere) override;
	void *toBinary (void *toHere) override;
	size_t getBinarySize () override;
	void set (int val);
	MyDB_IntAttVal ();
	~MyDB_IntAttVal ();

private:

	int value;
};

class MyDB_DoubleAttVal;
typedef shared_ptr <MyDB_DoubleAttVal> MyDB_DoubleAttValPtr;

// a double attributes
class MyDB_DoubleAttVal : public MyDB_AttVal {

public:

	int toInt () override;
	double toDouble () override;
	string toString () override;
	bool toBool () override;
	void fromString (string &fromMe) override;
	void *fromBinary (void *fromHere) override;
	void *toBinary (void *toHere) override;
	size_t getBinarySize () override;
	void set (double val);
	MyDB_DoubleAttVal ();
	~MyDB_DoubleAttVal ();

private:

	double value;
};

class MyDB_StringAttVal;
typedef shared_ptr <MyDB_StringAttVal> MyDB_StringAttValPtr;

// a string attribute
class MyDB_StringAttVal : public MyDB_AttVal {

public:

	int toInt () override;
	double toDouble () override;
	string toString () override;
	bool toBool () override;
	void fromString (string &fromMe) override;
	void *fromBinary (void *fromHere) override;
	void *toBinary (void *toHere) override;
	size_t getBinarySize () override;
	void set (string val);
	MyDB_StringAttVal ();
	~MyDB_StringAttVal ();

private:

	string value;
};

class MyDB_BoolAttVal;
typedef shared_ptr <MyDB_BoolAttVal> MyDB_BoolAttValPtr;

// a boolean attributes
class MyDB_BoolAttVal : public MyDB_AttVal {

public:

	int toInt () override;
	double toDouble () override;
	string toString () override;
	bool toBool () override;
	void fromString (string &fromMe) override;
	void *fromBinary (void *fromHere) override;
	void *toBinary (void *toHere) override;
	size_t getBinarySize () override;
	void set (bool val);
	MyDB_BoolAttVal ();
	~MyDB_BoolAttVal ();

private:

	bool value;
};

#endif
