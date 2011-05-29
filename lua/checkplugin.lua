-- checkplugin.lua

-- Checks the nominated plugin is installed

function do_plugin_check_now (id, name)

  local me  -- who am I? plugin or main world script?
  local location  -- location to attempt to load plugin from
  
  -- allow for being called from main world script
  if GetPluginID () == "" then
    me = "world script"
    location = GetInfo (60)
  else
    me = GetPluginName () .. " plugin"
    location =  GetPluginInfo(GetPluginID (), 20)
  end -- if
 
  -- first check if installed
  if not IsPluginInstalled (id) then
    ColourNote ("white", "green", "Plugin '" .. name .. "' not installed. Attempting to install it...") 
    LoadPlugin (location .. name .. ".xml") 
  
    if IsPluginInstalled (id) then
      ColourNote ("white", "green", "Success!") 
   
    -- here if still not installed
    else      
      ColourNote ("white", "red", string.rep ("-", 80))
      ColourNote ("white", "red", "Plugin '" .. name .. "' not installed. Please download and install it.") 
      ColourNote ("white", "red", "It is required for the correct operation of the " .. me)
      ColourNote ("white", "red", string.rep ("-", 80))
      return  -- skip enabled check
    end -- if not installed
  end -- plugin was not installed
   

  -- now make sure enabled (suggested by Fiendish - version 4.74+ )

  if not GetPluginInfo(id, 17) then
      ColourNote ("white", "green", "Plugin '" .. name .. "' not enabled. Attempting to enable it...")
      EnablePlugin(id, true)
      if GetPluginInfo(id, 17) then
         ColourNote ("white", "green", "Success!") 
      else
         ColourNote ("white", "red", string.rep ("-", 80))
         ColourNote ("white", "red", "Plugin '" .. name .. "' not enabled. Please make sure it can be enabled.")
         ColourNote ("white", "red", "It is required for the correct operation of the " .. me)
         ColourNote ("white", "red", string.rep ("-", 80))        
      end -- if
   end  -- if not enabled
   
end -- do_plugin_check_now


function checkplugin (id, name)

  if GetOption ("enable_timers") ~= 1 then
    ColourNote ("white", "red", "WARNING! Timers not enabled. Plugin dependency checks will not work properly.")
  end -- if timers disabled

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
    ColourNote ("white", "green", "Success!") 
    return ppi
  end

  ColourNote ("white", "red", string.rep ("-", 80))
  ColourNote ("white", "red", "Plugin '" .. name .. "' not installed. Please download and install it.") 
  ColourNote ("white", "red", string.rep ("-", 80))
  
  return nil
end -- function load_ppi 