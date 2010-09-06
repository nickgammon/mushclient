-- show_loaded.lua

--[[ 

Shows each plugin as it is loaded.

To enable this, add to:

  File menu -> Global Preferences -> Lua -> Preliminary Code 

  ... this line:
   
require "show_loaded"
  

--]]

if GetPluginID () == "" then
  ColourNote ("gray", "", "Initializing main world script ...")
else
  ColourNote ("gray", "", string.format ("Loading plugin '%s' (%s) version %0.2f ...",
    GetPluginInfo ( GetPluginID (), 7),
    GetPluginInfo ( GetPluginID (), 1),
    GetPluginInfo ( GetPluginID (), 19)))
end -- if
