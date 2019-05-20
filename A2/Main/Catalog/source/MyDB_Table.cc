
#ifndef TABLE_C
#define TABLE_C

#include "MyDB_Table.h"

MyDB_Table :: MyDB_Table (string name, string storageLocIn) {
	tableName = name;
	storageLoc = storageLocIn;
	last = -1;
}

MyDB_Table :: MyDB_Table (string name, string storageLocIn, MyDB_SchemaPtr mySchemaIn) {
	tableName = name;
	storageLoc = storageLocIn;
	mySchema = mySchemaIn;
	last = -1;
}

MyDB_Table :: ~MyDB_Table () {}

string &MyDB_Table :: getName () {
	return tableName;
}

string &MyDB_Table :: getStorageLoc () {
	return storageLoc;
}

map <string, MyDB_TablePtr> MyDB_Table :: getAllTables (MyDB_CatalogPtr fromMe) {

	map <string, MyDB_TablePtr> returnVal;

	// get all of the tables
        vector <string> myTables;
	fromMe->getStringList ("tables", myTables);

	// extract each of the tables from the catalog
	for (string s : myTables) {
		MyDB_TablePtr temp = make_shared <MyDB_Table> ();
		temp->fromCatalog (s, fromMe);	
		returnVal[s] = temp;
	}

	// outta here!!
	return returnVal;
}

MyDB_Table :: MyDB_Table () {}

int MyDB_Table :: lastPage () {
	return last;
}

void MyDB_Table :: setLastPage (size_t toMe) {
	last = (int) toMe;	
}

bool MyDB_Table :: fromCatalog (string tableNameIn, MyDB_CatalogPtr catalog) {
	
	// get the storage location
	tableName = tableNameIn;
        if (!catalog->getString (tableName + ".fileName", storageLoc)) {
		return false;
	}

	// and get the schema
	mySchema = make_shared <MyDB_Schema> ();
	mySchema->fromCatalog (tableName, catalog);

	// get the size
        catalog->getInt (tableName + ".lastPage", last);

	return true;
}

void MyDB_Table :: putInCatalog (MyDB_CatalogPtr catalog) {

        // get the list of tables
        vector <string> myTables;
        catalog->getStringList ("tables", myTables);

        // add the new table in, if not there
	bool inthere = false;	
	for (string s : myTables) {
		if (s == tableName)
			inthere = true;
	}

	if (!inthere) {
        	myTables.push_back (tableName);
        	catalog->putStringList ("tables", myTables);
	}

	// remember the storage location
        catalog->putString (tableName + ".fileName", storageLoc);

	// remember the last page in the file
        catalog->putInt (tableName + ".lastPage", last);

	// and add the schema in 
	mySchema->putInCatalog (tableName, catalog);	
}

MyDB_SchemaPtr MyDB_Table :: getSchema () {
	return mySchema;
}

std::ostream& operator<<(std::ostream& os, const MyDB_Table printMe) {
	os << "name: " << printMe.tableName << "; file: " << printMe.storageLoc << "; schema: " << printMe.mySchema;
    	return os;
}

std::ostream& operator<<(std::ostream& os, const MyDB_TablePtr printMe) {
	if (printMe == nullptr) {
		os << "<null>";
	} else {
		os << "name: " << printMe->tableName << "; file: " << printMe->storageLoc << "; schema: " << printMe->mySchema;
	}
    	return os;
}

#endif

