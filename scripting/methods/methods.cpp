#include "stdafx.h"
#include "..\..\MUSHclient.h"

//=============================================================
// IMPORTANT - when adding methods here remember to add the 'glue'
//             routine in lua_methods.cpp and add it to the library table
//   - see comments at the end of this file
//=============================================================


// ==============================================================================

// These are the methods exposed to external programs

// Now in separate files named: methods_xxxx.cpp


/*

  ======================================================================

  When adding a new script function, remember to do this as well:

  1. Add the Lua "glue" routine to lua_methods.cpp.
  
  2. Add the name of the function to the table in  functionlist.cpp 

  3. Add to the function documentation

  4. Add to release notes.

  5. Add to the help "contents" file: mushclient.cnt


  (please leave this comment at the bottom where it will hopefully be seen).

  ======================================================================

*/


