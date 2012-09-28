// methods_database.cpp

// Stuff for SQLite3 database management

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\MUSHview.h"
#include "..\errors.h"

// Implements:

//    DatabaseChanges
//    DatabaseClose
//    DatabaseColumnName
//    DatabaseColumnNames
//    DatabaseColumnText
//    DatabaseColumnType
//    DatabaseColumnValue
//    DatabaseColumnValues
//    DatabaseColumns
//    DatabaseError
//    DatabaseExec
//    DatabaseFinalize
//    DatabaseGetField
//    DatabaseInfo
//    DatabaseLastInsertRowid
//    DatabaseList
//    DatabaseOpen
//    DatabasePrepare
//    DatabaseReset
//    DatabaseStep
//    DatabaseTotalChanges


//  Flags For File Open Operations
//  
//  #define SQLITE_OPEN_READONLY         0x00000001
//  #define SQLITE_OPEN_READWRITE        0x00000002
//  #define SQLITE_OPEN_CREATE           0x00000004
//  #define SQLITE_OPEN_DELETEONCLOSE    0x00000008
//  #define SQLITE_OPEN_EXCLUSIVE        0x00000010
//  #define SQLITE_OPEN_MAIN_DB          0x00000100
//  #define SQLITE_OPEN_TEMP_DB          0x00000200
//  #define SQLITE_OPEN_TRANSIENT_DB     0x00000400
//  #define SQLITE_OPEN_MAIN_JOURNAL     0x00000800
//  #define SQLITE_OPEN_TEMP_JOURNAL     0x00001000
//  #define SQLITE_OPEN_SUBJOURNAL       0x00002000
//  #define SQLITE_OPEN_MASTER_JOURNAL   0x00004000
//  #define SQLITE_OPEN_NOMUTEX          0x00008000
//  #define SQLITE_OPEN_FULLMUTEX        0x00010000


// MUSHclient database error codes (negative to not clash with SQLite)

#define DATABASE_ERROR_ID_NOT_FOUND             -1   // unknown database ID
#define DATABASE_ERROR_NOT_OPEN                 -2   // not opened
#define DATABASE_ERROR_HAVE_PREPARED_STATEMENT  -3   // already have a prepared statement
#define DATABASE_ERROR_NO_PREPARED_STATEMENT    -4   // no prepared statement yet
#define DATABASE_ERROR_NO_VALID_ROW             -5   // have not stepped to valid row
#define DATABASE_ERROR_DATABASE_ALREADY_EXISTS  -6   // already have a database of that disk name
#define DATABASE_ERROR_COLUMN_OUT_OF_RANGE      -7   // requested column out of range


/////////////////////////////////////////////////////////////////////////////
// DatabaseOpen  - open a database

// Note: filename ":memory:" opens an in-memory database

long CMUSHclientDoc::DatabaseOpen(LPCTSTR Name, LPCTSTR Filename, long Flags) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);

  tDatabase * pDatabase = NULL;

  if (it == m_Databases.end ())
    {
    pDatabase = new tDatabase;

    pDatabase->db = NULL;
    pDatabase->pStmt = NULL;
    pDatabase->bValidRow = false;
    pDatabase->db_name = Filename;
    pDatabase->iColumns = 0;

    }
  else 
    {
    if (it->second->db_name == Filename)
      return SQLITE_OK;      // OK to re-use same database
    else
      return DATABASE_ERROR_DATABASE_ALREADY_EXISTS;   // database already exists under this id but a different disk name
    }

	int rc = sqlite3_open_v2 (Filename, &pDatabase->db, Flags, NULL);

  // add to map if opened OK
  if (rc == SQLITE_OK && pDatabase->db != NULL)
    m_Databases [Name] = pDatabase;
  else
    delete pDatabase;

  return rc;
}   // end of CMUSHclientDoc::DatabaseOpen

/////////////////////////////////////////////////////////////////////////////
// DatabaseClose  - close a database

long CMUSHclientDoc::DatabaseClose(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return DATABASE_ERROR_ID_NOT_FOUND;      // database not found

  if  (it->second->db == NULL)
    return DATABASE_ERROR_NOT_OPEN;          // database not open

  if (it->second->pStmt)        // finalize any outstanding statement
    sqlite3_finalize(it->second->pStmt);

  int rc = sqlite3_close(it->second->db);

  delete it->second;       // free up memory used by structure

  m_Databases.erase (it);  // remove from map

  return rc;               // return code from close
}   // end of CMUSHclientDoc::DatabaseClose

/////////////////////////////////////////////////////////////////////////////
// DatabasePrepare  - prepare (compile) an SQL statement

long CMUSHclientDoc::DatabasePrepare(LPCTSTR Name, LPCTSTR Sql) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return DATABASE_ERROR_ID_NOT_FOUND;       // database not found

  if  (it->second->db == NULL)
    return DATABASE_ERROR_NOT_OPEN;           // database not open

  if  (it->second->pStmt != NULL)
    return DATABASE_ERROR_HAVE_PREPARED_STATEMENT;     // already have prepared statement

  const char *pzTail;

  it->second->bValidRow = false;  // no valid row yet
  it->second->iColumns = 0;

  int rc = sqlite3_prepare_v2 (it->second->db, Sql, -1, &it->second->pStmt, &pzTail);

  // for future validation that columns are in range
  if (rc == SQLITE_OK)
     it->second->iColumns = sqlite3_column_count(it->second->pStmt);

  return rc;

}   // end of CMUSHclientDoc::DatabasePrepare

/////////////////////////////////////////////////////////////////////////////
// DatabaseFinalize  - finalize (discard) an SQL statement

long CMUSHclientDoc::DatabaseFinalize(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return DATABASE_ERROR_ID_NOT_FOUND;       // database not found

  if  (it->second->db == NULL)
    return DATABASE_ERROR_NOT_OPEN;           // database not open

  if  (it->second->pStmt == NULL)
    return DATABASE_ERROR_NO_PREPARED_STATEMENT;  // do not have prepared statement

  int rc = sqlite3_finalize(it->second->pStmt);  // finished with statement

  it->second->pStmt = NULL;     // show not in use
  it->second->bValidRow = false;  // no valid row
  it->second->iColumns = 0;   // no columns


	return rc;
}    // end of CMUSHclientDoc::DatabaseFinalize


/////////////////////////////////////////////////////////////////////////////
// DatabaseColumns  - find column count from a prepared SQL statement

// note: don't need to step first, the prepare gives us the count
long CMUSHclientDoc::DatabaseColumns(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return DATABASE_ERROR_ID_NOT_FOUND;           // database not found

  if  (it->second->db == NULL)
    return DATABASE_ERROR_NOT_OPEN;               // database not open

  if  (it->second->pStmt == NULL)
    return DATABASE_ERROR_NO_PREPARED_STATEMENT;  // do not have prepared statement

  return sqlite3_column_count(it->second->pStmt);  // column count

}   // end of CMUSHclientDoc::DatabaseColumns

/////////////////////////////////////////////////////////////////////////////
// DatabaseStep  - step through (execute) a prepared SQL statement

long CMUSHclientDoc::DatabaseStep(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return DATABASE_ERROR_ID_NOT_FOUND;           // database not found

  if  (it->second->db == NULL)
    return DATABASE_ERROR_NOT_OPEN;               // database not open

  if  (it->second->pStmt == NULL)
    return DATABASE_ERROR_NO_PREPARED_STATEMENT;  // do not have prepared statement

  int rc = sqlite3_step (it->second->pStmt);  // step into next row

  it->second->bValidRow = rc == SQLITE_ROW;   // show if we have a row or not

  return rc;
}    // end of CMUSHclientDoc::DatabaseStep

/////////////////////////////////////////////////////////////////////////////
// DatabaseError  - return English code from last database error

BSTR CMUSHclientDoc::DatabaseError(LPCTSTR Name) 
{
	CString strResult;

  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    strResult = Translate ("database id not found");   // database not found
  else if (it->second->db == NULL)
    strResult = Translate ("database not open");       // database not open
  else
    {
    switch (sqlite3_errcode (it->second->db))
      {
      case SQLITE_ROW:  
            strResult = Translate ("row ready");   
            break;

      case SQLITE_DONE: 
            strResult = Translate ("finished");    
            break;

      case DATABASE_ERROR_ID_NOT_FOUND: 
            strResult = Translate ("database id not found");          
            break;

      case DATABASE_ERROR_NOT_OPEN: 
            strResult = Translate ("database not open");              
            break;

      case DATABASE_ERROR_HAVE_PREPARED_STATEMENT: 
            strResult = Translate ("already have prepared statement");
            break;

      case DATABASE_ERROR_NO_PREPARED_STATEMENT: 
            strResult = Translate ("do not have prepared statement"); 
            break;

      case DATABASE_ERROR_NO_VALID_ROW: 
            strResult = Translate ("do not have a valid row");        
            break;

      case DATABASE_ERROR_DATABASE_ALREADY_EXISTS: 
            strResult = Translate ("database already exists under a different disk name"); 
            break;

      case DATABASE_ERROR_COLUMN_OUT_OF_RANGE: 
            strResult = Translate ("column count out of valid range"); 
            break;

      default:
          strResult = sqlite3_errmsg (it->second->db);
          break;
      } // end of switch
    }

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::DatabaseError


/////////////////////////////////////////////////////////////////////////////
// DatabaseColumnName  - find column name from a prepared SQL statement

// note: don't need to step first, the prepare gives us the names
BSTR CMUSHclientDoc::DatabaseColumnName(LPCTSTR Name, long Column) 
{
	CString strResult;

  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL &&      // and is open
      it->second->pStmt != NULL &&   // and we have a prepared statement
      Column >= 1 &&
      Column <= it->second->iColumns)
    {
    const char * p = sqlite3_column_name (it->second->pStmt, Column - 1);

    if (p)
      strResult = p;
    }

	return strResult.AllocSysString();
}  // end of CMUSHclientDoc::DatabaseColumnName

/////////////////////////////////////////////////////////////////////////////
// DatabaseColumnText  - find column data (as text) from last step

// WARNING: Lua version implemented separately in lua_methods.cpp
//          due to conversion problems with BSTR and some code pages

BSTR CMUSHclientDoc::DatabaseColumnText(LPCTSTR Name, long Column) 
{
	CString strResult;

  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL &&      // and is open
      it->second->pStmt != NULL &&   // and we have a prepared statement
      it->second->bValidRow &&       // and we stepped to a valid row
      Column >= 1 &&
      Column <= it->second->iColumns)
    {
    const unsigned char * p = sqlite3_column_text (it->second->pStmt, Column - 1);

    if (p)
      strResult = p;
    }

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::DatabaseColumnText

/////////////////////////////////////////////////////////////////////////////
// DatabaseColumnText  - find column value from last step

// WARNING: Lua version implemented separately in lua_methods.cpp
//          due to conversion problems with BSTR and some code pages

VARIANT CMUSHclientDoc::DatabaseColumnValue(LPCTSTR Name, long Column) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);
  vaResult.vt = VT_NULL;

  tDatabaseMapIterator it = m_Databases.find (Name);

  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL &&      // and is open
      it->second->pStmt != NULL &&   // and we have a prepared statement
      it->second->bValidRow &&       // and we stepped to a valid row
      Column >= 1 &&
      Column <= it->second->iColumns)
    {
    int iType = sqlite3_column_type(it->second->pStmt, Column - 1);

    switch (iType)
      {
      case SQLITE3_TEXT:
      case SQLITE_BLOB:
      default:
        {
        CString strResult;
        const unsigned char * p = sqlite3_column_text (it->second->pStmt, Column - 1);
        if (p)
          strResult = p;
        SetUpVariantString (vaResult, strResult); 
        }
        break;

      case SQLITE_NULL:
        vaResult.vt = VT_EMPTY;
        break;

      case SQLITE_INTEGER:
        {
        int iResult =  sqlite3_column_int  (it->second->pStmt, Column - 1);
        SetUpVariantLong (vaResult, iResult); 
        }
        break;

      case SQLITE_FLOAT:
        {
        double fResult =  sqlite3_column_double (it->second->pStmt, Column - 1);
        SetUpVariantDouble (vaResult, fResult); 
        }
        break;

      }  // end of switch

    }  // end of something valid



	return vaResult;
}     // end of CMUSHclientDoc::DatabaseColumnValue


/////////////////////////////////////////////////////////////////////////////
// DatabaseColumnType  - find column type from last step


// #define SQLITE_INTEGER  1
// #define SQLITE_FLOAT    2
// #define SQLITE3_TEXT    3
// #define SQLITE_BLOB     4
// #define SQLITE_NULL     5


long CMUSHclientDoc::DatabaseColumnType(LPCTSTR Name, long Column) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return DATABASE_ERROR_ID_NOT_FOUND;           // database not found

  if  (it->second->db == NULL)
    return DATABASE_ERROR_NOT_OPEN;               // database not open

  if  (it->second->pStmt == NULL)
    return DATABASE_ERROR_NO_PREPARED_STATEMENT;  // do not have prepared statement

  if  (!it->second->bValidRow)
    return DATABASE_ERROR_NO_VALID_ROW;           // do not have a valid row

  if  (Column < 1 || Column > it->second->iColumns)
    return DATABASE_ERROR_COLUMN_OUT_OF_RANGE;    // column count out of valid range

  return sqlite3_column_type(it->second->pStmt, Column - 1);  // column type
}     // end of CMUSHclientDoc::DatabaseColumnType


/////////////////////////////////////////////////////////////////////////////
// DatabaseTotalChanges  - find total changes to the database

long CMUSHclientDoc::DatabaseTotalChanges(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return DATABASE_ERROR_ID_NOT_FOUND;       // database not found

  if  (it->second->db == NULL)
    return DATABASE_ERROR_NOT_OPEN;           // database not open

  return sqlite3_total_changes(it->second->db); 

}    // end of CMUSHclientDoc::DatabaseTotalChanges


/////////////////////////////////////////////////////////////////////////////
// DatabaseChanges  - find last changes to the database

long CMUSHclientDoc::DatabaseChanges(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return DATABASE_ERROR_ID_NOT_FOUND;      // database not found

  if  (it->second->db == NULL)
    return DATABASE_ERROR_NOT_OPEN;          // database not open

  return sqlite3_changes(it->second->db); 
}   // end of CMUSHclientDoc::DatabaseChanges


/////////////////////////////////////////////////////////////////////////////
// DatabaseLastInsertRowid  - find last row ID inserted (converted to string)

BSTR CMUSHclientDoc::DatabaseLastInsertRowid(LPCTSTR Name) 
{
	CString strResult;

  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL)
    {
    strResult.Format("%I64d", sqlite3_last_insert_rowid(it->second->db));
    }

	return strResult.AllocSysString();
}    // end of CMUSHclientDoc::DatabaseLastInsertRowid

/////////////////////////////////////////////////////////////////////////////
// DatabaseList  - find list of databases

VARIANT CMUSHclientDoc::DatabaseList() 
{
  COleSafeArray sa;   // for array list

  long iCount = 0;
  
  // put the databases into the array
  if (!m_Databases.empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, m_Databases.size ());

    for (tDatabaseMapIterator it = m_Databases.begin (); 
         it != m_Databases.end ();
         it++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (it->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	return sa.Detach ();
}    // end of CMUSHclientDoc::DatabaseList

/////////////////////////////////////////////////////////////////////////////
// DatabaseInfo  - find info about a database

VARIANT CMUSHclientDoc::DatabaseInfo(LPCTSTR Name, long InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
  	return vaResult;

  tDatabase * pDatabase = it->second;

  switch (InfoType)
    {
    case 1:  SetUpVariantString  (vaResult, pDatabase->db_name.c_str ()); break; // disk name
    case 2:  SetUpVariantBool    (vaResult, pDatabase->pStmt != NULL);   break;  // valid prepared statement
    case 3:  SetUpVariantBool    (vaResult, pDatabase->bValidRow);       break;  // valid row returned from last step
    case 4:  SetUpVariantLong    (vaResult, pDatabase->iColumns);        break;  // number of columns 

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch


	return vaResult;
}   // end of CMUSHclientDoc::DatabaseInfo


/////////////////////////////////////////////////////////////////////////////
// DatabaseExec  - execute an SQL statement

long CMUSHclientDoc::DatabaseExec(LPCTSTR Name, LPCTSTR Sql) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return DATABASE_ERROR_ID_NOT_FOUND;              // database not found

  if  (it->second->db == NULL)
    return DATABASE_ERROR_NOT_OPEN;                  // database not open

  if  (it->second->pStmt != NULL)
    return DATABASE_ERROR_HAVE_PREPARED_STATEMENT;   // already have prepared statement

  it->second->bValidRow = false;  // no valid row yet
  it->second->iColumns = 0;

  return sqlite3_exec (it->second->db, Sql, NULL, NULL, NULL);
}   // end of CMUSHclientDoc::DatabaseExec

/////////////////////////////////////////////////////////////////////////////
// DatabaseColumnNames  - get table of column names from last prepared statement

// returns an array of column names
VARIANT CMUSHclientDoc::DatabaseColumnNames(LPCTSTR Name) 
{
  COleSafeArray sa;   // for array list

 
  tDatabaseMapIterator it = m_Databases.find (Name);

  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL &&      // and is open
      it->second->pStmt != NULL)  // and we have a prepared statement
    {
    // put the column names into the array
    if (it->second->iColumns > 0)    // cannot create empty dimension
      {
      sa.CreateOneDim (VT_VARIANT, it->second->iColumns);

      for (long i = 0; i < it->second->iColumns; i++)
         {

         CString strResult;

         const char * p = sqlite3_column_name (it->second->pStmt, i);

         if (p)
           strResult = p;

         COleVariant v (strResult);
         sa.PutElement (&i, &v);
         }

      } // end of having at least one

    }

	return sa.Detach ();
}  // end of CMUSHclientDoc::DatabaseColumnNames

/////////////////////////////////////////////////////////////////////////////
// DatabaseColumnValues  - get table of column values from last step

// WARNING: Lua version implemented separately in lua_methods.cpp
//          due to conversion problems with BSTR and some code pages

// returns an array of column values
VARIANT CMUSHclientDoc::DatabaseColumnValues(LPCTSTR Name) 
{
  COleSafeArray sa;   // for array list

 
  tDatabaseMapIterator it = m_Databases.find (Name);

  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL &&      // and is open
      it->second->pStmt != NULL &&   // and we have a prepared statement
      it->second->bValidRow)         // and have a valid row
    {
    // put the column values into the array
    if (it->second->iColumns > 0)    // cannot create empty dimension
      {
      sa.CreateOneDim (VT_VARIANT, it->second->iColumns);

      for (long i = 0; i < it->second->iColumns; i++)
         {

        int iType = sqlite3_column_type(it->second->pStmt, i);
        COleVariant v ;

        switch (iType)
          {
          case SQLITE3_TEXT:
          case SQLITE_BLOB:
          default:
            {
            CString strResult;
            const unsigned char * p = sqlite3_column_text (it->second->pStmt, i);
            if (p)
              strResult = p;
            v = strResult;
            }
            break;

          case SQLITE_NULL:
            v.ChangeType (VT_NULL);
            break;

          case SQLITE_INTEGER:
            {
            long iResult =  sqlite3_column_int  (it->second->pStmt, i);
            v = iResult; 
            }
            break;

          case SQLITE_FLOAT:
            {
            double fResult =  sqlite3_column_double (it->second->pStmt, i);
            v = fResult; 
            }
            break;

          }  // end of switch
         sa.PutElement (&i, &v);

         }

      } // end of having at least one

    }

	return sa.Detach ();
}  // end of  CMUSHclientDoc::DatabaseColumnNames


/////////////////////////////////////////////////////////////////////////////
// DatabaseReset  - reset a prepared statement (go back to start)

long CMUSHclientDoc::DatabaseReset(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return DATABASE_ERROR_ID_NOT_FOUND;           // database not found

  if  (it->second->db == NULL)
    return DATABASE_ERROR_NOT_OPEN;               // database not open

  if  (it->second->pStmt == NULL)
    return DATABASE_ERROR_NO_PREPARED_STATEMENT;  // do not have prepared statement

  return sqlite3_reset(it->second->pStmt);  // reset statement

}  // end of  CMUSHclientDoc::DatabaseReset


/////////////////////////////////////////////////////////////////////////////
// DatabaseGetField  - get row 1 column 1 corresponding to an SQL statement

// WARNING: Lua version implemented separately in lua_methods.cpp
//          due to conversion problems with BSTR and some code pages

VARIANT CMUSHclientDoc::DatabaseGetField(LPCTSTR Name, LPCTSTR Sql) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);


  // prepare the SQL statement
  long rc = DatabasePrepare (Name, Sql);

  if (rc != SQLITE_OK)
    return vaResult;  // could not prepare statement, give up

  // step to get one row
  rc = DatabaseStep (Name);

  // if we got one, extract the value from column 1
  if (rc == SQLITE_ROW)
    vaResult = DatabaseColumnValue (Name, 1);

  // finalize the current statement
  DatabaseFinalize (Name);

  // return the result
	return vaResult;

}  // end of CMUSHclientDoc::DatabaseGetField

