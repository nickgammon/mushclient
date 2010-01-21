--[[

PLUGIN-to-PLUGIN-INTERFACE (PPI)

Author:  	Twisol 
Date:  3rd January 2010

Amendments: Nick Gammon
Date: 8th January 2010

Example of use:

SERVICE

-- require PPI module
require "ppi"

-- exposed function
function SomeMethodHere (a, b, c, d)
  -- do something with a, b, c, d
  return 1, 2, 3, 4
end

-- notify PPI of this function
ppi.Expose "SomeMethodHere"

-- Or, for anonymous functions:
ppi.Expose ("DoSomethingElse", function () print "hello" end)

CLIENT

-- require PPI module
require "ppi"

-- resolve dependencies
function OnPluginListChanged ()
  
  -- get PPI entries for all exposed function in this plugin
  my_service = ppi.Load ("15783160bde378741f9652d1")  -- plugin ID of service plugin

  if not my_service then
    Note ("Dependency plugin not installed!")
  end
  
end -- OnPluginListChanged

-- later on in plugin ...

-- call SomeMethodHere in other plugin, passing various data types, getting results

if my_service then
  w, x, y, z = my_service.SomeMethodHere (42, "Nick", true, { a = 63, b = 22 } )
end -- if service installed

NOTES
-----

ppi.Load returns a table with various values in it about the target plugin (see below
for what they are). For example, _name is the plugin name of the target plugin, and
_version is the version number of that plugin.

If ppi.Load returns no value (effectively, nil) then the target plugin was not installed.

Provided a non-nil result was returned, you can then call any exposed function in the 
target plugin. There is currently no mechanism for finding what functions are exposed, for
simplicity's sake. However it would be possible to make a service function that returned all
exposed functions. If service plugins evolve in functionality, checking the target plugin's
version (the _version variable) should suffice for making sure plugins are synchronized.

To avoid clashes in variable names, you cannot expose a function starting with an underscore.

Communication with the target plugin is by global variables set up by the Expose function, along
the lines of:

PPI_function_name_PPI_  (one for each exposed function)

Also:

PPI__returns__PPI_ is used for storing the returned values.

--]]

-- hide all except non-local variables
module (..., package.seeall)

-- for transferring variables
require "serialize"

-- PPI version
local V_MAJOR, V_MINOR, V_PATCH = 1, 1, 0
local VERSION = string.format ("%d.%d.%d", V_MAJOR, V_MINOR, V_PATCH)

-- called plugin uses this variable to store returned values
local RETURNED_VALUE_VARIABLE = "PPI__returns__PPI_"

-- For any function in our PPI table, try to call that in the target plugin
local PPI_meta = {
  __index = function (tbl, idx)
    if (idx:sub (1, 1) ~= "_") then
      return function(...)
          -- Call the method in the target plugin
          local status = CallPlugin (tbl._id, "PPI_" .. idx .. "_PPI_", serialize.save_simple {...})
          
          -- explain a bit if we failed
          if status ~= error_code.eOK then
            ColourNote ("white", "red", "Error calling " .. idx .. 
                        " in plugin " .. tbl._name .. 
                        " using PPI from " .. GetPluginName () ..
                        " (" .. error_desc [status] .. ")")
            check (status)
          end -- if
          
          -- call succeeded, get any returned values
          local returns = {}  -- table of returned values
          local s = GetPluginVariable(tbl._id, RETURNED_VALUE_VARIABLE) or "{}"
          local f = assert (loadstring ("t = " .. s))  -- convert serialized data back
          setfenv (f, returns) () -- load the returned values into 'returns'
          
          -- unpack returned values to caller
          return unpack (returns.t)
        end  -- generated function
      end -- not starting with underscore
    end  -- __index function
}  -- end PPI_meta table

-- PPI request resolver
local function PPI_resolver (func) 
  return function (s)  -- calling plugin serialized parameters into a single string argument
    local params = {}  -- table of parameters
    local f = assert (loadstring ("t = " .. s))  -- convert serialized data back
    setfenv (f, params) ()  -- load the parameters into 'params'
    
    -- call target function, get return values, serialize back into variable
    SetVariable(RETURNED_VALUE_VARIABLE, serialize.save_simple {func(unpack (params.t))})
  end -- generated function
  
end -- PPI_resolver

-- EXPOSED FUNCTIONS

-- We "load" a plugin by checking it exists, and creating a table saving the
-- target plugin ID etc., and have a metatable which will handle function calls
function Load (plugin_id)
  if IsPluginInstalled (plugin_id) then
    return setmetatable (
      { _id = plugin_id,   -- so we know which plugin to call
        _name = GetPluginInfo (plugin_id, 1),
        _author = GetPluginInfo (plugin_id, 2),
        _filename = GetPluginInfo (plugin_id, 6),
        _enabled = GetPluginInfo (plugin_id, 17),
        _version = GetPluginInfo (plugin_id, 18),
        _required_version = GetPluginInfo (plugin_id, 19),
        _directory = GetPluginInfo (plugin_id, 20),
        _PPI_V_MAJOR = V_MAJOR,  -- version info
        _PPI_V_MINOR = V_MINOR,
        _PPI_V_PATCH = V_PATCH,
        _PPI_VERSION = VERSION,
       }, 
       PPI_meta)  -- everything except the above will generate functions
  else
    return nil, "Plugin ID " .. plugin_id .. " not installed"  -- in case you assert
  end -- if 
end -- function Load 

-- Used by a plugin to expose methods to other plugins
-- Each exposed function will be added to global namespace as PPI_<name>_PPI_
function Expose (name, func)
  assert (type (func or _G [name]) == "function", "Function " .. name .. " does not exist.")
  _G ["PPI_" .. name .. "_PPI_"] = PPI_resolver (func or _G [name])
end -- function Expose
