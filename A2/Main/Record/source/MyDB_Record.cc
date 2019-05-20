
#ifndef RECORD_CC
#define RECORD_CC

#include "MyDB_Record.h"
#include "MyDB_Schema.h"
#include <iostream>
#include <string.h>

using namespace std;

char *MyDB_Record :: findsymbol (char val, char *input) {
	while (*input != val) {
		input++;
	}
	return input + 1;
}

func MyDB_Record :: compileComputation (string compileMe) {
	char *str = (char *) compileMe.c_str ();
	return compileHelper (str).first;
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: compileHelper(char * &vals) {
	
	// search for one of the infix symbols
	while (true) {

		if (vals[0] == 0) {
			cout << "Reached end of string while parsing.\n";
			exit (1);
		}

		// not equal
		if (vals[0] == '!' && vals[1] == '=') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return neq (lres, rres);

		// not
		} else if (vals[0] == '!') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the result
			auto res = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return nott (res);
			
		// or
		} else if (vals[0] == '|' && vals[1] == '|') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return orr (lres, rres);

		// plus
		} else if (vals[0] == '+') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return plus (lres, rres);

		// and
		} else if (vals[0] == '&' && vals[1] == '&') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return andd (lres, rres);

		// equals
		} else if (vals[0] == '=' && vals[1] == '=') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return eq (lres, rres);

		// greater than
		} else if (vals[0] == '>') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return gt (lres, rres);

		// less than
		} else if (vals[0] == '<') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return lt (lres, rres);

		// times
		} else if (vals[0] == '*') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return times (lres, rres);

		// divide
		} else if (vals[0] == '/') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return divide (lres, rres);

		// minus
		} else if (vals[0] == '-') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return minus (lres, rres);

		// unary minus
		} else if (vals[0] == 'u' && vals[1] == 'm') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the result
			auto res = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return unaryMinus (res);

		// not equal
		} else if (vals[0] == '!' && vals[1] == '=') {

			// find the l-paren
			vals = findsymbol ('(', vals);

			// find the left result
			auto lres = compileHelper (vals);
			
			// and the comma
			vals = findsymbol (',', vals);

			// find the right result
			auto rres = compileHelper (vals);
			
			// find the r-paren
			vals = findsymbol (')', vals);
			
			// outta here!
			return neq (lres, rres);

		} else if (vals[0] == '[') {

			// find the right bracket
			vals++;
			int cnt = 0;
			for (; vals[cnt] != ']'; cnt++);

			// copy the string over
			char name[cnt + 1];
			for (cnt = 0; vals[cnt] != ']'; cnt++) {
				name[cnt] = vals[cnt];
			}	
			name[cnt] = 0;

			// find the ]
			vals = findsymbol (']', vals);
	
			// and get that attribute
			return fromData (name);		

		} else if (strncmp (vals, "int", 3) == 0) {

			vals = findsymbol ('[', vals);
			int val = stoi (vals);
			vals = findsymbol (']', vals);

			// remember this value
			MyDB_IntAttValPtr temp = make_shared <MyDB_IntAttVal> ();
			scratch.push_back (temp);
			temp->set (val);

			// returns a lambda that computes the result
			return make_pair ([temp] {return temp;}, make_shared <MyDB_IntAttType> ());

		} else if (strncmp (vals, "double", 6) == 0) {

			vals = findsymbol ('[', vals);
			double val = stod (vals);
			vals = findsymbol (']', vals);

			// remember this value
			MyDB_DoubleAttValPtr temp = make_shared <MyDB_DoubleAttVal> ();
			scratch.push_back (temp);
			temp->set (val);

			// returns a lambda that computes the result
			return make_pair ([temp] {return temp;}, make_shared <MyDB_DoubleAttType> ());

		} else if (strncmp (vals, "bool", 4) == 0) {

			vals = findsymbol ('[', vals);
			bool val = false;
			if (strncmp (vals, "true", 4) == 0) {
				val = true;
			}
			vals = findsymbol (']', vals);

			// remember this value
			MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
			scratch.push_back (temp);
			temp->set (val);

			// returns a lambda that computes the result
			return make_pair ([temp] {return temp;}, make_shared <MyDB_BoolAttType> ());

		} else if (strncmp (vals, "string", 6) == 0) {

			vals = findsymbol ('[', vals);

			// find the right bracket
			int cnt = 0;
			for (; vals[cnt] != ']'; cnt++);

			// copy the string over
			char name[cnt];
			for (cnt = 0; vals[cnt] != ']'; cnt++) {
				name[cnt] = vals[cnt];
			}	
			name[cnt] = 0;

			// find the ]
			vals = findsymbol (']', vals);
	
			// remember this value
			MyDB_StringAttValPtr temp = make_shared <MyDB_StringAttVal> ();
			scratch.push_back (temp);
			temp->set (name);

			// returns a lambda that computes the result
			return make_pair ([temp] {return temp;}, make_shared <MyDB_StringAttType> ());
			
		} else {
			vals++;
		}
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: fromData (string attName) {

	// just return a particular attribute
	auto whichAtt = mySchema->getAttByName (attName);
	return make_pair ([this, whichAtt] {return values[whichAtt.first];}, whichAtt.second);		
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: plus (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->promotableToInt () && rhs.second->promotableToInt ()) {
		MyDB_IntAttValPtr temp = make_shared <MyDB_IntAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toInt () + rhs.first ()->toInt ()); return temp;},
			make_shared <MyDB_IntAttType> ());

	// otherwise, if both sides can be cast upwards to be doubles, then do so
	} else if (lhs.second->promotableToDouble () && rhs.second->promotableToDouble ()) {
		MyDB_DoubleAttValPtr temp = make_shared <MyDB_DoubleAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toDouble () + rhs.first ()->toDouble ()); return temp;},
			make_shared <MyDB_DoubleAttType> ());

	// otherwise, if both sides can be cast upwards to be strings, then do so
	} else if (lhs.second->promotableToString () && rhs.second->promotableToString ()) {
		MyDB_StringAttValPtr temp = make_shared <MyDB_StringAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toString () + rhs.first ()->toString ()); return temp;},
			make_shared <MyDB_StringAttType> ());

	} else {
		cout << "This is bad... cannot do anything with the plus.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: minus (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->promotableToInt () && rhs.second->promotableToInt ()) {
		MyDB_IntAttValPtr temp = make_shared <MyDB_IntAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toInt () - rhs.first ()->toInt ()); return temp;},
			make_shared <MyDB_IntAttType> ());

	// otherwise, if both sides can be cast upwards to be doubles, then do so
	} else if (lhs.second->promotableToDouble () && rhs.second->promotableToDouble ()) {
		MyDB_DoubleAttValPtr temp = make_shared <MyDB_DoubleAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toDouble () - rhs.first ()->toDouble ()); return temp;},
			make_shared <MyDB_DoubleAttType> ());
	
	} else {
		cout << "This is bad... cannot do anything with the minus.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: unaryMinus (pair <func, MyDB_AttTypePtr> lhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->promotableToInt ()) {
		MyDB_IntAttValPtr temp = make_shared <MyDB_IntAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs] {temp->set (-lhs.first ()->toInt ()); return temp;},
			make_shared <MyDB_IntAttType> ());

	// otherwise, if both sides can be cast upwards to be doubles, then do so
	} else if (lhs.second->promotableToDouble ()) {
		MyDB_DoubleAttValPtr temp = make_shared <MyDB_DoubleAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs] {temp->set (-lhs.first ()->toDouble ()); return temp;},
			make_shared <MyDB_DoubleAttType> ());
	
	} else {
		cout << "This is bad... cannot do anything with the unary minus.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: times (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->promotableToInt () && rhs.second->promotableToInt ()) {
		MyDB_IntAttValPtr temp = make_shared <MyDB_IntAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toInt () * rhs.first ()->toInt ()); return temp;},
			make_shared <MyDB_IntAttType> ());

	// otherwise, if both sides can be cast upwards to be doubles, then do so
	} else if (lhs.second->promotableToDouble () && rhs.second->promotableToDouble ()) {
		MyDB_DoubleAttValPtr temp = make_shared <MyDB_DoubleAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toDouble () * rhs.first ()->toDouble ()); return temp;},
			make_shared <MyDB_DoubleAttType> ());

	} else {
		cout << "This is bad... cannot do anything with the times.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: divide (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->promotableToInt () && rhs.second->promotableToInt ()) {
		MyDB_IntAttValPtr temp = make_shared <MyDB_IntAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toInt () / rhs.first ()->toInt ()); return temp;},
			make_shared <MyDB_IntAttType> ());

	// otherwise, if both sides can be cast upwards to be doubles, then do so
	} else if (lhs.second->promotableToDouble () && rhs.second->promotableToDouble ()) {
		MyDB_DoubleAttValPtr temp = make_shared <MyDB_DoubleAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toDouble () / rhs.first ()->toDouble ()); return temp;},
			make_shared <MyDB_DoubleAttType> ());

	} else {
		cout << "This is bad... cannot do anything with the divide.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: gt (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->promotableToInt () && rhs.second->promotableToInt ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toInt () > rhs.first ()->toInt ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	// otherwise, if both sides can be cast upwards to be doubles, then do so
	} else if (lhs.second->promotableToDouble () && rhs.second->promotableToDouble ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toDouble () > rhs.first ()->toDouble ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	// otherwise, if both sides can be cast upwards to be strings, then do so
	} else if (lhs.second->promotableToString () && rhs.second->promotableToString ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toString () > rhs.first ()->toString ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	} else {
		cout << "This is bad... cannot do anything with the >.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: lt (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->promotableToInt () && rhs.second->promotableToInt ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toInt () < rhs.first ()->toInt ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	// otherwise, if both sides can be cast upwards to be doubles, then do so
	} else if (lhs.second->promotableToDouble () && rhs.second->promotableToDouble ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toDouble () < rhs.first ()->toDouble ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	// otherwise, if both sides can be cast upwards to be strings, then do so
	} else if (lhs.second->promotableToString () && rhs.second->promotableToString ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toString () < rhs.first ()->toString ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	} else {
		cout << "This is bad... cannot do anything with the >.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: eq (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->promotableToInt () && rhs.second->promotableToInt ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toInt () == rhs.first ()->toInt ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	// otherwise, if both sides can be cast upwards to be doubles, then do so
	} else if (lhs.second->promotableToDouble () && rhs.second->promotableToDouble ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toDouble () == rhs.first ()->toDouble ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	} else if (lhs.second->isBool () && rhs.second->isBool ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toBool () == rhs.first ()->toBool ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	// otherwise, if both sides can be cast upwards to be strings, then do so
	} else if (lhs.second->promotableToString () && rhs.second->promotableToString ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toString () == rhs.first ()->toString ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	} else {
		cout << "This is bad... cannot do anything with the >.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: neq (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->promotableToInt () && rhs.second->promotableToInt ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toInt () != rhs.first ()->toInt ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	} else if (lhs.second->isBool () && rhs.second->isBool ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toBool () != rhs.first ()->toBool ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	// otherwise, if both sides can be cast upwards to be doubles, then do so
	} else if (lhs.second->promotableToDouble () && rhs.second->promotableToDouble ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toDouble () != rhs.first ()->toDouble ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	// otherwise, if both sides can be cast upwards to be strings, then do so
	} else if (lhs.second->promotableToString () && rhs.second->promotableToString ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toString () != rhs.first ()->toString ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	} else {
		cout << "This is bad... cannot do anything with the >.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: orr (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->isBool () && rhs.second->isBool ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toBool () || rhs.first ()->toBool ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	} else {
		cout << "This is bad... cannot do or on non booleans.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: andd (pair <func, MyDB_AttTypePtr> lhs, pair <func, MyDB_AttTypePtr> rhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->isBool () && rhs.second->isBool ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs, rhs] {temp->set (lhs.first ()->toBool () && rhs.first ()->toBool ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	} else {
		cout << "This is bad... cannot do or on non booleans.\n";
		exit (1);
	}
}

pair <func, MyDB_AttTypePtr> MyDB_Record :: nott (pair <func, MyDB_AttTypePtr> lhs) {

	// if both sides can be cast upwards to be ints, then do so
	if (lhs.second->isBool ()) {
		MyDB_BoolAttValPtr temp = make_shared <MyDB_BoolAttVal> ();
		scratch.push_back (temp);

		// returns a lambda that computes the result
		return make_pair ([temp, lhs] {temp->set (!lhs.first ()->toBool ()); return temp;},
			make_shared <MyDB_BoolAttType> ());

	} else {
		cout << "This is bad... cannot do not on non boolean.\n";
		exit (1);
	}
}

size_t MyDB_Record :: getBinarySize () {
	size_t total = 0;
	for (MyDB_AttValPtr temp : values) {
		total += temp->getBinarySize ();
	}	
	return total;
}

void *MyDB_Record :: toBinary (void *toHere) {
	for (MyDB_AttValPtr temp : values) {
		toHere = temp->toBinary (toHere);
	}		
	return toHere;
}

void *MyDB_Record :: fromBinary (void *fromHere) {
	for (MyDB_AttValPtr temp : values) {
		fromHere = temp->fromBinary (fromHere);
	}		
	return fromHere;
}

void MyDB_Record :: fromString (string res) {	
	int i = 0;
        for (int pos = 0; pos < (int) res.size (); pos = (int) res.find ("|", pos + 1) + 1) {
                string temp = res.substr (pos, res.find ("|", pos + 1) - pos);
		values[i++]->fromString (temp);
        }
}

std::ostream& operator<<(std::ostream& os, const MyDB_Record printMe) {
	for (MyDB_AttValPtr temp : printMe.values) {
		os << temp->toString () << "|";
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const MyDB_RecordPtr printMe) {
	if (printMe == nullptr)
		return os;
	for (MyDB_AttValPtr temp : printMe->values) {
		os << temp->toString () << "|";
	}
	return os;


}

MyDB_Record :: MyDB_Record (MyDB_SchemaPtr mySchemaIn) {
	mySchema = mySchemaIn;
	for (auto &val : mySchema->getAtts ()) {
		values.push_back (val.second->createAtt ());	
	}
}

MyDB_Record :: ~MyDB_Record () {}

#endif
