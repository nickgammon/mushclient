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