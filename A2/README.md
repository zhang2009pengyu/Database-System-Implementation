Group Member: Siqi Huang : sh71 Yiwen Chen: yc92

Design Description: For a table, all write and read is through
TableReaderWriter and we use a vector to store the PageReadWriters of
all pages in the table.

For every page in the table file, there is a header, which is used to
record some necessary information such as pagetype, offset to the last
record and the beginning position of the first record.

Initializing the tableReaderWriter is to initialize the
PageReaderWriter in the vector for every existing page.

WRITE: When loading the table from a text file (lines of record), we
create a empty record. For every line, in sequence, load them to this
record and call the method append() in the tableReaderWriter.  The
append method is adding record to the last page in the table, actually
through calling append method in pageReaderWriter. First call append
of the lastpage's PageReaderWriter, if it's full we need to call
method createNewEmptyPage to create a new pageReadWriter and append
record in it.

In the createNewEmptyPage method, we need to first create a page
through getpage() called by buffermanager and return a pagehandle. And
then through the method getBytes() called by pagehandle to get to the
fisrt bytes in the page and initialize the header of it. Then create a
pageReadWriter and add it to the vector.

READ: The way to get every record in the table is to use a single
record object to process all the records in sequence.  The iterator in
the table is actually a pageIterator. The class pageIterator is used
to process every record of current page. First we get the current
pageReadWriter and then call getIterator of pageReadWriter to get the
current page Iterator.  Method hasNext and getNext is used to move the
iterator and load every record into this single object.  hasnext() is
first to check the current page to see if it has next record(through
calling hasNext() in the pageIterator) and the check the next
page. When check next page, we need to get a new page Iterator. And
getnext() is to get the next record in the current page.


