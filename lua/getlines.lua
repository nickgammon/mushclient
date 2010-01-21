-- getlines.lua
-- getlines iterator - iterates over a string and returns one item per line

function getlines (str)

  local pos = 0
  
  -- the for loop calls this for every iteration
  -- returning nil terminates the loop
  local function iterator (s)
  
    if not pos then
      return nil
    end -- end of string, exit loop
   
    local oldpos = pos + 1  -- step past previous newline
    pos = string.find (s, "\n", oldpos) -- find next newline
  
    if not pos then  -- no more newlines, return rest of string
      return string.sub (s, oldpos)
    end -- no newline
    
    return string.sub (s, oldpos, pos - 1)
    
  end -- iterator
  
  return iterator, str
end -- getlines

return getlines

--[=[

 Example of use:
 
 require "getlines"
 
 test = [[
every good
boy
deserves 
fruit]]

for l in getlines (test) do
  print ('"' .. l .. '"')
end -- for

--]=]
