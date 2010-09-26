//  methods_arrays.cpp

// Array processing

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\errors.h"


// Implements:

//    ArrayClear
//    ArrayCount
//    ArrayCreate
//    ArrayDelete
//    ArrayDeleteKey
//    ArrayExists
//    ArrayExport
//    ArrayExportKeys
//    ArrayGet
//    ArrayGetFirstKey
//    ArrayGetLastKey
//    ArrayImport
//    ArrayKeyExists
//    ArrayListAll
//    ArrayListKeys
//    ArrayListValues
//    ArraySet
//    ArraySize


/*

  arrays test:

arraycreate "a"
arrayimport "a", "nick,blah,helen,aaaa", ","
note arrayexport ("a", "|")
note arrayget ("a", "nick")
arrayset "a", "nick", "fish"
arrayset "a", "c", "c-value"
arrayset "a", "z", "oh, nothing"

arrayset "a", "1", "number 1"
arrayset "a", "11", "number 3"
arrayset "a", "2", "number 2"
  
arrayset "a", "nick,has a \\in his house", " with,fish,and\\,chips "
note arrayexport ("a", ",")
arraycreate "b"
arrayimport "b", arrayexport ("a", ","), "," 
world.debug "arrays"
      
Note ArrayGetFirstKey ("a")
Note ArrayGetLastKey ("a")


note arrayget ("a", "nick")

dim keyList

keyList = World.ArrayListKeys ("a")

If Not IsEmpty (keyList) Then
  
  For Each v In keyList 
    world.note "key = " & v
  Next

End If

arraydeletekey "a", "nick"
note arrayexport ("a", "|")

note "a exists = " & arrayexists ("a")
note "b exists = " & arrayexists ("b")
note "a/helen exists = " & arraykeyexists ("a", "helen")
note "a/jack exists = " & arraykeyexists ("a", "jack")

note arrayexportkeys ("a", ",")

note "count of arrays = " & arraycount
note "size of a = " & arraysize ("a")


ArrayCreate "spells"
ArrayCreate "onespell"

ArrayClear "spells"
ArraySet "spells", "dragonskin", "45"

ArrayClear "onespell"
ArraySet "onespell", "mana",  "45"
ArraySet "onespell", "wearoff", "Your flesh sheds its draconian aspects."
ArraySet "onespell", "hitvict", "Your flesh changes to emulate the scaly skin of a dragon."

ArraySet "spells", "dragonskin", ArrayExport ("onespell", "~")

ArrayClear "onespell"
ArraySet "onespell", "mana",  "75"
ArraySet "onespell", "wearoff", "The ethereal funnel about you ceases to exist."
ArraySet "onespell", "hitvict", "An aura surrounds you, channeling violent energies in your direction!"

ArraySet "spells", "ethereal funnel", ArrayExport ("onespell", "~")

Note ArrayExport ("spells", "=")

world.debug "arrays"

ArrayClear "onespell"
ArrayImport "onespell", ArrayGet ("spells",  "ethereal funnel"), "~"

Note "hitvict message for ethereal funnel is: " & _
   ArrayGet ("onespell", "hitvict")



arraycreate "a"
arrayclear "a"
arrayset "a", "nick,has a \in his house", " with,fish,and\,chips "
note "'" &  arrayexport ("a", ",") & "'"

x = arrayexport ("a", ",")
arraycreate "b"
arrayimport "b", x, ","
note "'" &  arrayexport ("b", ",") & "'"

  */


// create a new array
long CMUSHclientDoc::ArrayCreate(LPCTSTR Name) 
{

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it != GetArrayMap ().end ())
    return eArrayAlreadyExists;

  tStringToStringMap * m = new tStringToStringMap;

  GetArrayMap ().insert (make_pair (Name, m));

  return eOK;

  } // end of CMUSHclientDoc::ArrayCreate

// delete an array by name
long CMUSHclientDoc::ArrayDelete(LPCTSTR Name) 
{

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return eArrayDoesNotExist;

  it->second->clear ();  // remove all entries

  delete it->second;     // delete the pointer

  GetArrayMap ().erase (it);  // erase from map of arrays

	return eOK;
  }  // end of CMUSHclientDoc::ArrayDelete

// list all known arrays into an array of variants

VARIANT CMUSHclientDoc::ArrayListAll() 
{
  COleSafeArray sa;   // for array list

  long iCount = 0;
  
  // put the arrays into the array
  if (!GetArrayMap ().empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, GetArrayMap ().size ());

    for (tStringMapOfMaps::iterator it = GetArrayMap ().begin (); 
         it != GetArrayMap ().end ();
         it++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (it->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	return sa.Detach ();
}   // end of CMUSHclientDoc::ArrayListAll

// list the keys for a particular array

VARIANT CMUSHclientDoc::ArrayListKeys(LPCTSTR Name) 
{
  COleSafeArray sa;   // for array list

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return sa.Detach ();   // empty array if array does not exist

  long iCount = 0;
  
  // put the keys into the array
  if (!it->second->empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, it->second->size ());

    for (tStringToStringMap::iterator i = it->second->begin (); 
         i != it->second->end ();
         i++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (i->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	return sa.Detach ();
} // end of CMUSHclientDoc::ArrayListKeys

// lists the values (not the keys) in a particular array

VARIANT CMUSHclientDoc::ArrayListValues(LPCTSTR Name) 
{
  COleSafeArray sa;   // for array list

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return sa.Detach ();   // empty array if array does not exist

  long iCount = 0;
  
  // put the keys into the array
  if (!it->second->empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, it->second->size ());

    for (tStringToStringMap::iterator i = it->second->begin (); 
         i != it->second->end ();
         i++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (i->second.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	return sa.Detach ();
}   // end of CMUSHclientDoc::ArrayListValues

// import from a delimited string into an array
// eg.  world.ArrayImport "myarray", "nick,blah,helen,aaaa", ","

// importing is tricky because of the blasted "escaped delimiter"
// so, what I am going to do is replace the escaped delimiter with something else
// then fix import using the ordinary delimiter, then fix up the mess afterwards

long CMUSHclientDoc::ArrayImport(LPCTSTR Name, LPCTSTR Values, LPCTSTR Delimiter) 
{
  // delimiter had better be a single character, other than backslash
  string sDelimiter (Delimiter);

  if (sDelimiter.size () != 1 || sDelimiter == "\\")
     return eBadDelimiter;

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return eArrayDoesNotExist;

  vector<string> v;
  int iDuplicates = 0;

  string sEscapedDelimiter = "\\" + sDelimiter;
  string sValues (Values);  
  string sTemp;

  if (sValues.find (sEscapedDelimiter) != string::npos)
    {
    int iTemp;

    // find a character not in the string
    for (iTemp = 1; iTemp <= 255; iTemp++)
      {
      sTemp = string (1, iTemp);
      if (sValues.find (sTemp) == string::npos)
        break;  // this character isn't used!
      }

    if (iTemp > 255)
      return eCannotImport;

    sValues = FindAndReplace (sValues, sEscapedDelimiter, sTemp);

    } // end of having at least one escaped delimiter in the string

  StringToVector (sValues, v, sDelimiter, false); // don't trim spaces

  if (v.size () & 1)
    return eArrayNotEvenNumberOfValues;

  // insert pairs (key, value) into designated map
  for (vector<string>::iterator i = v.begin (); i != v.end (); )
    {
    string sKey = *i++;
    string sValue = *i++;

    // fix up backslashes and delimiters
    sKey = FindAndReplace (sKey, "\\\\", "\\");
    if (!sTemp.empty ())
      sKey = FindAndReplace (sKey, sTemp, sDelimiter);
    sValue = FindAndReplace (sValue, "\\\\", "\\");
    if (!sTemp.empty ())
      sValue = FindAndReplace (sValue, sTemp, sDelimiter);

    pair<tStringToStringMap::iterator, bool> status = 
        it->second->insert (make_pair (sKey, sValue));

    if (!status.second)
      {
      status.first->second = sValue;
      iDuplicates++;
      }
    }

  if (iDuplicates)
    return eImportedWithDuplicates;

	return eOK;
} // end of CMUSHclientDoc::ArrayImport

// get an element from an array by key
VARIANT CMUSHclientDoc::ArrayGet(LPCTSTR Name, LPCTSTR Key) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return vaResult;     // empty

  tStringToStringMap::iterator i = it->second->find (Key);

  if (i == it->second->end ())
    return vaResult;     // empty

  SetUpVariantString (vaResult, i->second.c_str ());
  
	return vaResult;
  }  // end of CMUSHclientDoc::ArrayGet

// set an array item by key - overwriting is allowed but warned in the return value
long CMUSHclientDoc::ArraySet(LPCTSTR Name, LPCTSTR Key, LPCTSTR Value) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return eArrayDoesNotExist;   

  pair<tStringToStringMap::iterator, bool> status = 
      it->second->insert (make_pair (Key, Value));

  if (!status.second)
    {
    status.first->second = Value;
    return eSetReplacingExistingValue;
    }

	return eOK;
} // end of CMUSHclientDoc::ArraySet

// exports an entire array as a delimited string
// it is an error if the delimiter is in the array keys or values

VARIANT CMUSHclientDoc::ArrayExport(LPCTSTR Name, LPCTSTR Delimiter) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    {
    SetUpVariantLong (vaResult, eArrayDoesNotExist);
	  return vaResult;
    }

  // empty array gives empty string
  if (it->second->empty ())
    {
    SetUpVariantString (vaResult, "");
    return vaResult;
    }

  // delimiter had better be a single character, other than backslash
  string sDelimiter (Delimiter);

  if (sDelimiter.size () != 1 || sDelimiter == "\\")
    {
    SetUpVariantLong (vaResult, eBadDelimiter);
    return vaResult;
    }

  string sResult;
  int iCount = 0;

  for (tStringToStringMap::iterator i = it->second->begin ();
       i != it->second->end ();
       i++)
         {
         string sKey (i->first);
         string sData (i->second);

         // replace backslashes by two of them
         sKey = FindAndReplace (sKey, "\\", "\\\\");
         // replace delimiter by backslash, delimiter
         sKey = FindAndReplace (sKey, sDelimiter, "\\" + sDelimiter);

         // replace backslashes by two of them
         sData = FindAndReplace (sData, "\\", "\\\\");
         // replace delimiter by backslash, delimiter
         sData = FindAndReplace (sData, sDelimiter, "\\" + sDelimiter);

         sResult += sKey + sDelimiter + sData;
         if (++iCount < it->second->size ())
          sResult += sDelimiter;
         }  // end of doing each one


  SetUpVariantString (vaResult, sResult.c_str ());
	return vaResult;
} // end of CMUSHclientDoc::ArrayExport

// export the keys but not the values as a delimited string
VARIANT CMUSHclientDoc::ArrayExportKeys(LPCTSTR Name, LPCTSTR Delimiter) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    {
    SetUpVariantLong (vaResult, eArrayDoesNotExist);
	  return vaResult;
    }

  // empty array gives empty string
  if (it->second->empty ())
    {
    SetUpVariantString (vaResult, "");
    return vaResult;
    }

  // delimiter had better be a single character, other than backslash
  string sDelimiter (Delimiter);

  if (sDelimiter.size () != 1 || sDelimiter == "\\")
    {
    SetUpVariantLong (vaResult, eBadDelimiter);
    return vaResult;
    }

  string sResult;
  int iCount = 0;

  for (tStringToStringMap::iterator i = it->second->begin ();
       i != it->second->end ();
       i++)
     {
     string sKey (i->first);

     // replace backslashes by two of them
     sKey = FindAndReplace (sKey, "\\", "\\\\");
     // replace delimiter by backslash, delimiter
     sKey = FindAndReplace (sKey, sDelimiter, "\\" + sDelimiter);

     sResult += sKey;
     if (++iCount < it->second->size ())
      sResult += sDelimiter;
     }  // end of doing each one

  SetUpVariantString (vaResult, sResult.c_str ());
	return vaResult;
} // end of CMUSHclientDoc::ArrayExportKeys

// clear existing contents from an array
// eg. world.ArrayClear "myarray"

long CMUSHclientDoc::ArrayClear(LPCTSTR Name) 
{

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return eArrayDoesNotExist;

  it->second->clear ();  // remove all entries

	return eOK;
} // end of CMUSHclientDoc::ArrayClear

// delete a keyed item from an array
//  eg. world.ArrayDeleteKey "myarray", "spells"

long CMUSHclientDoc::ArrayDeleteKey(LPCTSTR Name, LPCTSTR Key) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return eArrayDoesNotExist;   

  tStringToStringMap::iterator i = it->second->find (Key);

  if (i == it->second->end ())
    return eKeyDoesNotExist;     

  it->second->erase (i);

  return eOK;
  
} // end of CMUSHclientDoc::ArrayDeleteKey

// quick test to see if an array exists
BOOL CMUSHclientDoc::ArrayExists(LPCTSTR Name) 
{

  return GetArrayMap ().find (Name) != GetArrayMap ().end ();

  } // end of CMUSHclientDoc::ArrayExists

// quick test to see if a key exists
BOOL CMUSHclientDoc::ArrayKeyExists(LPCTSTR Name, LPCTSTR Key) 
{

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return FALSE;   

  return it->second->find (Key) != it->second->end ();

} // end of CMUSHclientDoc::ArrayKeyExists

// return count of all arrays
long CMUSHclientDoc::ArrayCount() 
{
	return GetArrayMap ().size ();
} // end of CMUSHclientDoc::ArrayCount

// return count of elements in a particular array
long CMUSHclientDoc::ArraySize(LPCTSTR Name) 
{

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return 0;   

	return it->second->size ();
}   // end of CMUSHclientDoc::ArraySize

VARIANT CMUSHclientDoc::ArrayGetFirstKey(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return vaResult;     // empty

  tStringToStringMap::iterator i = it->second->begin ();

  if (i == it->second->end ())
    return vaResult;     // empty

  SetUpVariantString (vaResult, i->first.c_str ());
  
	return vaResult;
}   // end of CMUSHclientDoc::ArrayGetFirstKey

VARIANT CMUSHclientDoc::ArrayGetLastKey(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return vaResult;     // empty

  tStringToStringMap::reverse_iterator i = it->second->rbegin ();

  if (i == it->second->rend ())
    return vaResult;     // empty

  SetUpVariantString (vaResult, i->first.c_str ());
  
	return vaResult;
}   // end of CMUSHclientDoc::ArrayGetLastKey

