--
--  tprint.lua

--[[

For debugging what tables have in them, prints recursively

See forum thread:  http://www.gammon.com.au/forum/?id=4903

eg.

require "tprint"

   tprint (GetStyleInfo (20))

--]]

function tprint (t, indent, done)
  -- in case we run it standalone
  local Note = Note or print
  local Tell = Tell or io.write
  
  -- show strings differently to distinguish them from numbers
  local function show (val)
    if type (val) == "string" then
      return '"' .. val .. '"'
    else
      return tostring (val)
    end -- if
  end -- show
  -- entry point here
  done = done or {}
  indent = indent or 0
  for key, value in pairs (t) do
    Tell (string.rep (" ", indent)) -- indent it
    if type (value) == "table" and not done [value] then
      done [value] = true
      Note (show (key), ":");
      tprint (value, indent + 2, done)
    else
      Tell (show (key), "=")
      print (show (value))
    end
  end
end

return tprint
