Sqlite3 built as follows:

1. Download recent version from http://www.sqlite.org/sqlite-amalgamation-3070800.zip

2. Unzip the file

3. Copy the following files to the mushclient source "sqlite3" directory:

sqlite3.c
sqlite3.h
sqlite3ext.h


3. Edit: sqlite3.c and add the following lines to the start of it:

// These first few lines added by NJG
// version 3.7.8
# pragma warning (disable : 4018)  // signed/unsigned mismatch
# pragma warning (disable : 4022)  // pointer mismatch for actual parameter x
# pragma warning (disable : 4047)  // 'void ** ' differs in levels of indirection from 'long *'
# pragma warning (disable : 4049)  // terminating line number emission
# pragma warning (disable : 4056)  // overflow in floating-point constant arithmetic
# pragma warning (disable : 4100)  // unreferenced formal parameter
# pragma warning (disable : 4115)  // named type definition in parentheses
# pragma warning (disable : 4127)  // conditional expression is constant
# pragma warning (disable : 4132)  // const object should be initialized
# pragma warning (disable : 4244)  // conversion from 'int ' to 'char ', possible loss of data
# pragma warning (disable : 4305)  // truncation from 'int ' to 'short '
# pragma warning (disable : 4701)  // 'xxx' may be used without having been initialized
# pragma warning (disable : 4702)  // unreachable code
# pragma warning (disable : 4706)  // assignment within conditional expression
# pragma warning (disable : 4756)  // overflow in constant arithmetic
# pragma warning (disable : 4761)  // integral size mismatch in argument; conversion supplied

#define SQLITE_SOUNDEX 1
#define SQLITE_ENABLE_FTS3 1
#define SQLITE_ENABLE_FTS3_PARENTHESIS 1
#define SQLITE_ENABLE_RTREE 1
#define SQLITE_USE_URI 1



Note: added patch:

http://www.sqlite.org/src/ci/59bb999c8b?sbs=0


@@ -379,10 +379,16 @@
           result = s / scale;
           result /= 1.0e+308;
         }else{
           result = s * scale;
           result *= 1.0e+308;
+        }
+      }else if( e>=342 ){
+        if( esign<0 ){
+          result = 0.0*s;
+        }else{
+          result = 1e308*1e308*s;  /* Infinity */
         }
       }else{
         /* 1.0e+22 is the largest power of 10 than can be
         ** represented exactly. */
         while( e%22 ) { scale *= 1.0e+1; e -= 1; }
