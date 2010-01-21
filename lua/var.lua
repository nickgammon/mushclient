-- var.lua
-- ----------------------------------------------------------
-- Accessing MUSHclient variables through the 'var' table.
-- See forum thread:
--  http://www.gammon.com.au/forum/?id=4904

--[[

  * Set a variable by assigning something to it.
  * Delete a variable by assigning nil to it.
  * Get a variable by retrieving its value, will return nil if the variable does not exist.
  
  Examples:

    var.target = "kobold"   -- set MUSHclient variable 'target' to kobold
    print (var.target)      -- print contents of MUSHclient variable

--]]

-- ----------------------------------------------------------

var = {}  -- variables table

setmetatable (var, 
 { 
 -- called to access an entry
 __index = 
 function (t, name) 
  return GetVariable (name) 
 end,
 
 -- called to change or delete an entry
 __newindex = 
 function (t, name, val) 
 local result
   if val == nil then -- nil deletes it
     result = DeleteVariable (name)
   else
     result = SetVariable (name, tostring (val)) 
   end 
   -- warn if they are using bad variable names
   if result == error_code.eInvalidObjectLabel then
     error ("Bad variable name '" .. name .. "'", 2)
   end
 end
 })
 
return var