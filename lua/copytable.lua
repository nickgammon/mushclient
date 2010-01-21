-- copytable.lua

--[[

Table copying functions. 

See: http://www.gammon.com.au/forum/?id=8042

Ideas by Shaun Biggs, David Haley, Nick Gammon

Date: 21st July 2007

This is intended to copy tables (make a real copy, rather than just the table
reference).

You can do a deep or shallow copy.

Shallow: Simply copies the keys and values. 
If a value is a table, you will get the same table as in the original.

Deep: Copies keys and values recursively.
If a value is a table, makes a copy of that table, and so on.

Deep copy based on: http://lua-users.org/wiki/CopyTable

Restrictions: Items must be "safe" to copy (eg. not file IO userdata). 

The deep copied tables share the same metatable as the original ones. 
To change this, change the line:

   return setmetatable(new_table, getmetatable(object))

to:

   return setmetatable(new_table, _copy (getmetatable(object))

Example:

t1 = {
  m = { a = 1, b = 2 },
  n = { c = 3, d = 4 },
  }

require "copytable"  -- load this file

t2 = copytable.shallow (t1)  -- shallow copy
t3 = copytable.deep (t1)  -- copies sub tables as well

--]]

module (..., package.seeall)

                     
function deep (object)
  local lookup_table = {}

  local function _copy (object)
    if type (object) ~= "table" then
      return object
    elseif lookup_table [object] then
      return lookup_table [object]
    end  -- if

    local new_table = {}
    lookup_table [object] = new_table

    for index, value in pairs (object) do
      new_table [_copy (index)] = _copy (value)
    end  -- for

    return setmetatable (new_table, getmetatable (object))
  end  -- function _copy

  return _copy (object)
end  -- function deepcopy

function shallow (t)
  assert (type (t) == "table", "You must specify a table to copy")
    
  local result = {}

  for k, v in pairs (t) do
    result [k] = v
  end -- for each table element

  -- copy the metatable
  return setmetatable (result, getmetatable (t))

end  -- function shallow
