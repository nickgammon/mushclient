-- declare.lua
-- See: http://www.gammon.com.au/forum/?id=7327
--
-- If you use this inside a function you cannot access global variables that have
-- not already been declared, and must declare all local variables

function force_declarations ()
  setfenv (2, setmetatable ({}, 
           {
           __index = function (t, n) 
                      error("variable '"..n.."' is not declared", 2) 
                      end, 
           __newindex = function (t, n, v) 
                      error("assign to undeclared variable '"..n.."'", 2) 
                      end })
           )
end -- force_declarations

return force_declarations

--[[

  Example of use:
  
  
  require "declare"
  
function test (x)
  -- capture any global variables we want
  local print = print
  
  -- after this we can't access global variables, and must declare local ones
  force_declarations ()
  
  -- must declare every variable now before we use it
  local a, b, c
  
  print (a)  
  a = 1
  b = 2
  c = x * 2
  print (c)
end -- test

test (1)

--]]
