-- pairsbykeys.lua
-- From Programming in Lua book, 2nd edition
-- Gives you an iterator that moves through an ordinary table (eg. string keys)
--  but sorted into key sequence.
-- It does that by copying the table keys into a temporary table and sorting that.

-- If you need to sort keys other than strings, see:

-- See: http://lua-users.org/wiki/SortedIteration

function pairsByKeys (t, f)
  local a = {}
  -- build temporary table of the keys
  for n in pairs (t) do 
    table.insert (a, n) 
  end
  table.sort (a, f)  -- sort using supplied function, if any
  local i = 0        -- iterator variable
  return function () -- iterator function
    i = i + 1
    return a[i], t[a[i]]
  end  -- iterator function
end -- pairsByKeys

return pairsByKeys

--[[

  
  -- This prints the math functions in random order
  for k, v in pairs (math) do
    print (k, v)
  end -- for
  
  require "pairsbykeys"
  
  -- This prints the math functions in key order
  for k, v in pairsByKeys (math) do
    print (k, v)
  end -- for
  
  --]]
  