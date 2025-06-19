// Wrapper to set SQLite compile-time options without modifying the amalgamated source
#define SQLITE_SOUNDEX 1
#define SQLITE_ENABLE_FTS3 1
#define SQLITE_ENABLE_FTS3_PARENTHESIS 1
#define SQLITE_ENABLE_RTREE 1
#define SQLITE_USE_URI 1
#include "sqlite3.c"
