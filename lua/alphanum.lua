-- alphanum.lua
--
-- Adapted somewhat from: http://www.davekoelle.com/files/alphanum.lua
-- Also see: http://www.davekoelle.com/alphanum.html
--
-- Implements a sort function that does a more "human readable" sort order.
-- It breaks the sort strings into "chunks" and then compares each one naturally,
-- depending on whether it is a string or a number (eg. z9.doc compares less than z20.doc)
-- It also does a case-insensitive compare (so "nick" and "Nick" come out together).

-- See also: http://www.gammon.com.au/forum/?id=8698

--[[
Example:

require "alphanum"

t={
"z18.doc","z19.doc","z2.doc","z16.doc","z17.doc",
"z1.doc","z101.doc","z102.doc","z11.doc","z12.doc",
"z13.doc","z14.doc","z15.doc","z20.doc","z3.doc",
"z4.doc","z5.doc","z6.doc","z10.doc","z100.doc",
"z7.doc","z8.doc","z9.doc", "Z9A.doc",
}

table.sort(t, alphanum (t))

for i=1, #t do
   print(t[i])
end

--]]

function alphanum (t)
  assert (type (t) == "table", "Must pass table to be sorted to alphanum")
  
  local function chunkString(str)
  local c = {}
  for a, b in tostring (str):gmatch("(%d*)(%D*)") do
     if a ~= "" then c[#c+1] = tonumber(a) end
     if b ~= "" then c[#c+1] = b end
    end
    return c
  end

  local chunks = {}
  -- build temporary table of the keys
  for i=1, #t do
   chunks [t [i]] = chunkString (t [i])
  end

  return function (a, b) -- return our sort comparison function
  
   -- lookup chunked information from previously-built table
   local achunks = chunks [a]
   local bchunks = chunks [b]
   
   for i = 1, math.min (#achunks, #bchunks) do
      local as, bs = achunks [i], bchunks [i]
      
      -- if one is a string, make them both strings
      if type (as) == "string" or type (bs) == "string"  then 
        as = (tostring (as)):upper ()
        bs = (tostring (bs)):upper ()
      end -- at least one is a string

      -- if they are equal, move onto the next chunk
      if as ~= bs then 
        return as < bs 
      end  -- if
   end  -- for each chunk
   
   -- still equal? the one with fewer chunks compares less
   return #achunks < #bchunks

  end  -- sort function

end -- alphanum

return alphanum
