--
-- check.lua
--
-- ----------------------------------------------------------
-- return-code checker for MUSHclient functions that return error codes
-- ----------------------------------------------------------
--
--[[

Call for those MUSHclient functions that return a result code (like eOK).
Not all functions return such a code.

eg.

require "check
  check (SetVariable ("abc", "def"))  --> works ok
  check (SetVariable ("abc-", "def")) --> The name of this object is invalid

--]]

function check (result)
  if result ~= error_code.eOK then
    error (error_desc [result] or 
           string.format ("Unknown error code: %i", result), 
           2) -- error level - whoever called this function
  end -- if
end -- function check 

return check