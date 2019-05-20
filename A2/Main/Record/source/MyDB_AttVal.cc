
#ifndef ATT_VAL_C
#define ATT_VAL_C

#include <iostream>
#include "MyDB_AttVal.h"
#include <string>
#include <string.h>

using namespace std;

MyDB_AttVal :: ~MyDB_AttVal () {}

int MyDB_IntAttVal :: toInt () {
	return value;
}

double MyDB_IntAttVal :: toDouble () {
	return (double) value;
}

string MyDB_IntAttVal :: toString () {
	return to_string (value);
}

void MyDB_IntAttVal :: fromString (string &fromMe) {
	value = stoi (fromMe);
}

bool MyDB_IntAttVal :: toBool () {
	cout << "Oops!  Can't convert int to bool";
	exit (1);
}

void *MyDB_IntAttVal :: fromBinary (void *fromHere) {
	value = *((int *) fromHere);
	return ((char *) fromHere) + sizeof (int);
}
void *MyDB_IntAttVal :: toBinary (void* toHere) {
	*((int *) toHere) = value;
	return ((char *) toHere) + sizeof (int);
}

void MyDB_IntAttVal :: set (int val) {
	value = val;
}

MyDB_IntAttVal :: MyDB_IntAttVal () {
	value = 0;
}

size_t MyDB_IntAttVal :: getBinarySize () {
	return sizeof (int);
}

MyDB_IntAttVal :: ~MyDB_IntAttVal () {}

int MyDB_DoubleAttVal :: toInt () {
	return value;
}

void MyDB_DoubleAttVal :: fromString (string &fromMe) {
	value = stod (fromMe);
}

double MyDB_DoubleAttVal :: toDouble () {
	return value;
}

string MyDB_DoubleAttVal :: toString () {
	return to_string (value);
}

bool MyDB_DoubleAttVal :: toBool () {
	cout << "Oops!  Can't convert int to bool";
	exit (1);
}

void *MyDB_DoubleAttVal :: fromBinary (void *fromHere) {
	value = *((double *) fromHere);
	return ((char *) fromHere) + sizeof (double);
}

void *MyDB_DoubleAttVal :: toBinary (void* toHere) {
	*((double *) toHere) = value;
	return ((char *) toHere) + sizeof (double);
}

void MyDB_DoubleAttVal :: set (double val) {
	value = val;
}

MyDB_DoubleAttVal :: MyDB_DoubleAttVal () {
	value = 0;
}

size_t MyDB_DoubleAttVal :: getBinarySize () {
	return sizeof (double);
}

MyDB_DoubleAttVal :: ~MyDB_DoubleAttVal () {}

int MyDB_StringAttVal :: toInt () {
	cout << "Oops!  Can't convert int to int";
	exit (1);
}

void MyDB_StringAttVal :: fromString (string &fromMe) {
	value = fromMe;
}

double MyDB_StringAttVal :: toDouble () {
	cout << "Oops!  Can't convert int to double";
	exit (1);
}

string MyDB_StringAttVal :: toString () {
	return value;
}

bool MyDB_StringAttVal :: toBool () {
	cout << "Oops!  Can't convert int to bool";
	exit (1);
}

void *MyDB_StringAttVal :: fromBinary (void *fromHere) {
	string temp ((char *) fromHere);
	value = temp;
	return ((char *) fromHere) + strlen ((char *) fromHere) + 1;
}

void *MyDB_StringAttVal :: toBinary (void* toHere) {
	memcpy ((char *) toHere, value.c_str (), strlen (value.c_str ()) + 1);
	return ((char *) toHere) + strlen (value.c_str ()) + 1;
}

void MyDB_StringAttVal :: set (string val) {
	value = val;
}

MyDB_StringAttVal :: MyDB_StringAttVal () {
	value = "";
}

size_t MyDB_StringAttVal :: getBinarySize () {
	return strlen (value.c_str ()) + 1;
}

MyDB_StringAttVal :: ~MyDB_StringAttVal () {}

int MyDB_BoolAttVal :: toInt () {
	cout << "Oops!  Can't convert bool to int";
	exit (1);
}

double MyDB_BoolAttVal :: toDouble () {
	cout << "Oops!  Can't convert bool to double";
	exit (1);
}

string MyDB_BoolAttVal :: toString () {
	if (value) {
		return "true";
	} else {
		return "false";
	}
}

void MyDB_BoolAttVal :: fromString (string &fromMe) {
	if (fromMe == "false") {
		value = false;
	} else if (fromMe == "true") {
		value = true;
	} else {
		cout << "Oops!  Bad string for boolean\n";
		exit (1);
	}
}

bool MyDB_BoolAttVal :: toBool () {
	return value;
}

void *MyDB_BoolAttVal :: fromBinary (void *fromHere) {
	if (*((char *) fromHere) == 1) {
		value = true;
	} else if (*((char *) fromHere) == 0) {
		value = true;
	} else {
		cout << "Bad value for boolean.\n";
		exit (1);
	}
	return ((char *) fromHere) + sizeof (char);
}

void *MyDB_BoolAttVal :: toBinary (void* toHere) {
	if (value) {
		*((char *) toHere) = 1;
	} else {
		*((char *) toHere) = 0;
	}
	return ((char *) toHere) + sizeof (char);
}

void MyDB_BoolAttVal :: set (bool val) {
	value = val;
}

MyDB_BoolAttVal :: MyDB_BoolAttVal () {
	value = false;
}

size_t MyDB_BoolAttVal :: getBinarySize () {
	return sizeof (char);
}

MyDB_BoolAttVal :: ~MyDB_BoolAttVal () {}

#endif
