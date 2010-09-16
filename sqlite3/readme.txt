Sqlite3 built as follows:

1. Download recent version from http://www.sqlite.org/sqlite-amalgamation-3_7_2.zip

2. Unzip and untar the file

3. Copy the following files to the mushclient source "sqlite3" directory:

sqlite3.c
sqlite3.h
sqlite3ext.h


3. Edit: sqlite3.c and add the following lines to the start of it:

// These first few lines added by NJG
// version 3.7.2
#  pragma warning(disable : 4022)  // pointer mismatch for actual parameter x
#  pragma warning(disable : 4047)  // 'void ** ' differs in levels of indirection from 'long *'
#  pragma warning(disable : 4049)  // terminating line number emission
#  pragma warning(disable : 4100)  // unreferenced formal parameter
#  pragma warning(disable : 4115)  // named type definition in parentheses
#  pragma warning(disable : 4127)  // conditional expression is constant
#  pragma warning(disable : 4132)  // const object should be initialized
#  pragma warning(disable : 4244)  // conversion from 'int ' to 'char ', possible loss of data
#  pragma warning(disable : 4305)  // truncation from 'int ' to 'short '
#  pragma warning(disable : 4761)  // integral size mismatch in argument; conversion supplied

#define SQLITE_SOUNDEX 1
#define SQLITE_ENABLE_FTS3 1
#define SQLITE_ENABLE_FTS3_PARENTHESIS 1
