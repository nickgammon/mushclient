--[[

  string_indexing.lua
  
  If you install this, then you can index into strings, like this:
  
  
    require "string_indexing"
    
    a = "nick"
    print (a [3])  --> c



--]]

getmetatable ("").__index = function (str, i)

  if (type (i) == "number") then
    return string.sub (str, i, i)  -- index into str
  end -- if

  return string [i]  -- fallback (eg. string.match)
end -- function
