LICENSE
-------

See:

  https://www.sqlite.org/copyright.html

All of the code and documentation in SQLite has been dedicated to the public domain by the authors.
...
Anyone is free to copy, modify, publish, use, compile, sell, or distribute the original SQLite code, 
either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means. 


Note that the SQLite3 source is unchanged except as noted below.


BUILDING A NEW VERSION
----------------------



Sqlite3 built as follows:

1. Download new amalgamated source code from https://www.sqlite.org/download.html

2. Unzip the file

3. Copy the following files to the mushclient source "sqlite3" directory:

sqlite3.c
sqlite3.h
sqlite3ext.h


3. Edit: sqlite3.c and add the following lines to the start of it:

#define SQLITE_SOUNDEX 1
#define SQLITE_ENABLE_FTS3 1
#define SQLITE_ENABLE_FTS3_PARENTHESIS 1
#define SQLITE_ENABLE_RTREE 1
#define SQLITE_USE_URI 1

4. If you get compile errors like C2059 'bad suffix on number' on older compilers, remove the LL suffix from constants.
For example, change:

    && i >= -2251799813685248LL && i < 2251799813685248LL);

to:

    && i >= -2251799813685248 && i < 2251799813685248);

This currently happens on four lines in the source in sqlite3.c.
(Visual Studio 6.0 uses a suffix of i64 or ui64 instead of LL or ULL).

