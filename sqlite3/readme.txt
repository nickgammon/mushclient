Sqlite3 built as follows:

1. Download recent version from http://www.sqlite.org/sqlite-amalgamation-3_6_22.zip

2. Unzip and untar the file

3. Copy the following files to the mushclient source "sqlite3" directory:

sqlite3.c
sqlite3.h
sqlite3ext.h


3. Edit: sqlite3.c and add the following lines to the start of it:

// These first few lines added by NJG
// version 3.6.22
#  pragma warning(disable: 4244)
#  pragma warning(disable: 4018)
#  pragma warning(disable: 4049)
#  pragma warning(disable: 4761)
#  pragma warning(disable: 4047)
#  pragma warning(disable: 4022)
#define SQLITE_SOUNDEX 1
#define SQLITE_ENABLE_FTS3 1
#define SQLITE_ENABLE_FTS3_PARENTHESIS 1
