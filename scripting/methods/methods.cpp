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



/*

-- Lua code to work out what functions are in what file:
   

require "commas"

funcs = {}

for _, v in ipairs (utils.functionlist ()) do
  funcs [v] = true
end -- for


-- this function is called for every found file
function load_file (name, stats)

  print (string.rep ("-", 40))
 
  local n = string.match (name, "[%w._]+$")
  print (n)

  print ""
  print ("// Implements:")
  print ""

  local f = assert (io.open (name, "r"))  -- open it
  local s = f:read ("*a")  -- read all of it
  f:close ()  -- close it
  
  local t = {}

  for w in string.gmatch (s, "CMUSHclientDoc::([%w_]+)") do
    if funcs [w] then
      table.insert (t, w)
    end -- if a script function
  end -- for

  table.sort (t)

  local done = {}

  for _, name in ipairs (t) do
     if not done [name] then
       print ("//   ", name)
     end -- if
     done [name] = true
  end -- for

  print ""

end -- load_file


scan_dir ("\\source\\mushclient\\scripting\\methods", load_file)

*/
