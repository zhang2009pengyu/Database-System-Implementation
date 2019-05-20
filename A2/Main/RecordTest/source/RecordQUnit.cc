
#ifndef RECORD_TEST_H
#define RECORD_TEST_H

#include "MyDB_AttType.h"  
#include "MyDB_BufferManager.h"
#include "MyDB_Catalog.h"  
#include "MyDB_Page.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_Record.h"
#include "MyDB_Table.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_Schema.h"
#include "QUnit.h"
#include <iostream>

int main () {

        QUnit::UnitTest qunit(cerr, QUnit::verbose);

	// create the catalog
	{

		// create a schema
		MyDB_SchemaPtr mySchema = make_shared <MyDB_Schema> ();
		mySchema->appendAtt (make_pair ("firstAtt", make_shared <MyDB_IntAttType> ()));
		mySchema->appendAtt (make_pair ("secAtt", make_shared <MyDB_IntAttType> ()));
		mySchema->appendAtt (make_pair ("thirdAtt", make_shared <MyDB_DoubleAttType> ()));
		mySchema->appendAtt (make_pair ("fourthAtt", make_shared <MyDB_StringAttType> ()));

		// create a pair of tables with this schema
		MyDB_TablePtr myTable = make_shared <MyDB_Table> ("coolTable", "coolTableFile", mySchema);
		MyDB_TablePtr yourTable = make_shared <MyDB_Table> ("awesomeTable", "awesomeTableFile", mySchema);

		// and write to the catalog
		MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> ("catFile");
		myTable->putInCatalog (myCatalog);
		yourTable->putInCatalog (myCatalog);
		
		cout << myTable << "\n";
		cout << yourTable << "\n";
		cout << "\n\n";
	}

	{
		// loop through all of the tables in there
		MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> ("catFile");
		map <string, MyDB_TablePtr> allTables = MyDB_Table :: getAllTables (myCatalog);
		for (auto e : allTables) {
			cout << e.second << "\n";
		}
		cout << "\n\n";
	}
	
	{

		// create a catalog
		MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> ("catFile");

		// now make a schema
		MyDB_SchemaPtr mySchema = make_shared <MyDB_Schema> ();
		mySchema->appendAtt (make_pair ("suppkey", make_shared <MyDB_IntAttType> ()));
		mySchema->appendAtt (make_pair ("name", make_shared <MyDB_StringAttType> ()));
		mySchema->appendAtt (make_pair ("address", make_shared <MyDB_StringAttType> ()));
		mySchema->appendAtt (make_pair ("nationkey", make_shared <MyDB_IntAttType> ()));
		mySchema->appendAtt (make_pair ("phone", make_shared <MyDB_StringAttType> ()));
		mySchema->appendAtt (make_pair ("acctbal", make_shared <MyDB_DoubleAttType> ()));
		mySchema->appendAtt (make_pair ("comment", make_shared <MyDB_StringAttType> ()));

		// use the schema to create a table
		MyDB_TablePtr myTable = make_shared <MyDB_Table> ("supplier", "supplier.bin", mySchema);
		MyDB_BufferManagerPtr myMgr = make_shared <MyDB_BufferManager> (1024, 16, "tempFile");
		MyDB_TableReaderWriter supplierTable (myTable, myMgr);

		// load it from a text file
		supplierTable.loadFromTextFile ("supplier.tbl");

		supplierTable.writeIntoTextFile ("supplier.tbl2");

		// put the supplier table into the catalog
		myTable->putInCatalog (myCatalog);
	}
	

	{

		// load up the table supplier table from the catalog
		MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> ("catFile");
		map <string, MyDB_TablePtr> allTables = MyDB_Table :: getAllTables (myCatalog);
		MyDB_BufferManagerPtr myMgr = make_shared <MyDB_BufferManager> (1024, 16, "tempFile");
		MyDB_TableReaderWriter supplierTable (allTables["supplier"], myMgr);

		// now, go to the 37th page and iterate over it
		MyDB_RecordPtr temp = supplierTable.getEmptyRecord ();
		MyDB_RecordIteratorPtr myIter = supplierTable[36].getIterator (temp);
		while (myIter->hasNext ()) {
			myIter->getNext ();
			cout << temp << "\n";
		}

		cout << "\n\n\n";
	}

	{
		// load up the table supplier table from the catalog
		MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> ("catFile");
		map <string, MyDB_TablePtr> allTables = MyDB_Table :: getAllTables (myCatalog);
		MyDB_BufferManagerPtr myMgr = make_shared <MyDB_BufferManager> (1024, 16, "tempFile");
		MyDB_TableReaderWriter supplierTable (allTables["supplier"], myMgr);

		// test the iterator by looping through all of the records in the file
		MyDB_RecordPtr temp = supplierTable.getEmptyRecord ();
		MyDB_RecordIteratorPtr myIter = supplierTable.getIterator (temp);

		// there should be 10000 records
		int counter = 0;
		while (myIter->hasNext ()) {
			myIter->getNext ();
			counter++;
	 	}
		QUNIT_IS_EQUAL (counter, 10000);
	}
		
	{
		// load up the table supplier table from the catalog
		MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> ("catFile");
		map <string, MyDB_TablePtr> allTables = MyDB_Table :: getAllTables (myCatalog);
		MyDB_BufferManagerPtr myMgr = make_shared <MyDB_BufferManager> (1024, 16, "tempFile");
		MyDB_TableReaderWriter supplierTable (allTables["supplier"], myMgr);

		// test the iterator by looping through all of the records in the file
		MyDB_RecordPtr temp = supplierTable.getEmptyRecord ();
		MyDB_RecordIteratorPtr myIter = supplierTable.getIterator (temp);

		// this will compute the square of the "suppkey" attribute
		func square = temp->compileComputation ("* ( [suppkey], [suppkey]) ");

		// this will check to make sure that the suppkey is less than 100
		func check = temp->compileComputation ("< ( [suppkey], int[100]) ");

		int total = 0;
		while (myIter->hasNext ()) {
			myIter->getNext ();
			if (check ()->toBool ()) {
				total += square ()->toInt ();
			}
	 	}

		QUNIT_IS_EQUAL (total, 328350);
	}


	int countOf67thPage = 0;
	{
		
		// load up the table supplier table from the catalog
		MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> ("catFile");
		map <string, MyDB_TablePtr> allTables = MyDB_Table :: getAllTables (myCatalog);
		MyDB_BufferManagerPtr myMgr = make_shared <MyDB_BufferManager> (1024, 16, "tempFile");
		MyDB_TableReaderWriter supplierTable (allTables["supplier"], myMgr);

		// count the records in the 67th page
		MyDB_RecordPtr temp = supplierTable.getEmptyRecord ();
		MyDB_RecordIteratorPtr myIter = supplierTable[66].getIterator (temp);
		while (myIter->hasNext ()) {
			myIter->getNext ();
			countOf67thPage++;	
		}

		// clear the 67th page
		supplierTable[66].clear ();

		// make sure the count is zero now
		int count = 0;
		myIter = supplierTable[66].getIterator (temp);
		while (myIter->hasNext ()) {
			myIter->getNext ();
			count++;
		}
		
		QUNIT_IS_EQUAL (count, 0);
	}

	int numCopied = 0;
	{

		// load up the table supplier table from the catalog
		MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> ("catFile");
		map <string, MyDB_TablePtr> allTables = MyDB_Table :: getAllTables (myCatalog);
		MyDB_BufferManagerPtr myMgr = make_shared <MyDB_BufferManager> (1024, 16, "tempFile");
		MyDB_TableReaderWriter supplierTable (allTables["supplier"], myMgr);

		// test the iterator by looping through all of the records in the file
		MyDB_RecordPtr temp = supplierTable.getEmptyRecord ();
		MyDB_RecordIteratorPtr myIter = supplierTable.getIterator (temp);

		// there should be less than 10000 records
		int counter = 0;
		while (myIter->hasNext ()) {
			myIter->getNext ();
			counter++;
	 	}
		QUNIT_IS_EQUAL (counter, 10000 - countOf67thPage);

		// now, copy the contents of the 100th page to the 99th
		supplierTable[98].clear ();
		myIter = supplierTable[99].getIterator (temp);
		cout << "These should be the same as:\n";
		while (myIter->hasNext ()) {
			myIter->getNext ();
			supplierTable[98].append (temp);
			cout << temp << "\n";
			numCopied++;
		}
		cout << "\n\n\n";
		
	}

	{

		// load up the table supplier table from the catalog
		MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> ("catFile");
		map <string, MyDB_TablePtr> allTables = MyDB_Table :: getAllTables (myCatalog);
		MyDB_BufferManagerPtr myMgr = make_shared <MyDB_BufferManager> (1024, 16, "tempFile");
		MyDB_TableReaderWriter supplierTable (allTables["supplier"], myMgr);

		// now, go to the 99th page and iterate over it
		MyDB_RecordPtr temp = supplierTable.getEmptyRecord ();
		MyDB_RecordIteratorPtr myIter = supplierTable[98].getIterator (temp);
		int counter = 0;
		cout << "These:\n";
		while (myIter->hasNext ()) {
			myIter->getNext ();
			cout << temp << "\n";
			counter++;
		}

		QUNIT_IS_EQUAL (counter, numCopied);
		cout << "\n\n\n";
	}

}

#endif
