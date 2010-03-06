-- checkplugin.xml

-- Checks the nominated plugin is installed

function do_plugin_check_now (id, name)

  if IsPluginInstalled (id) then
    return  -- all is well
  end -- plugin is installed

  ColourNote ("black", "yellow", "Warning! Plugin " .. name .. " has not been installed.")
  ColourNote ("black", "yellow", "It is required for the correct operation of the " ..
              GetPluginName () .. " plugin.")

end -- do_plugin_check_now



function checkplugin (id, name)
  -- give them time to load
  DoAfterSpecial (2, 
                  "do_plugin_check_now ('" .. id .. "', '" .. name .. "')", 
                  sendto.script)
end -- checkplugin

function load_ppi (id, name)
local PPI = require "ppi"

  local ppi = PPI.Load(id)  
  if ppi then
    return ppi
  end
  
  ColourNote ("white", "green", "Plugin '" .. name .. "' not installed. Attempting to install it...") 
  LoadPlugin (GetPluginInfo(GetPluginID (), 20) .. name .. ".xml") 
  
  ppi = PPI.Load(id)  -- try again
  if ppi then
    return ppi
  end

  ColourNote ("white", "red", string.rep ("-", 80))
  ColourNote ("white", "red", "Plugin '" .. name .. "' not installed. Please download and install it.") 
  ColourNote ("white", "red", string.rep ("-", 80))
  
  return nil
end -- function load_ppi 