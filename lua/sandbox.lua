-- sandbox.lua

--[[ 

MUSHclient sandbox (taken from versions 4.11 to 4.57)

To enable the sandbox for all Lua scripting add to:

   File menu -> Global Preferences -> Lua -> Preliminary Code 

   ... this line:
   
require "sandbox"

See: http://mushclient.com/security

Note that this sandbox only affects Lua, not other scripting languages.

--]]


trust_all_worlds = false    -- change to true to trust all the worlds
trust_all_plugins = false   -- change to true to trust all the plugins
warn_if_not_trusted = false -- change to true to show warnings

--[[

-- Lua initialization (sandbox) --> please read comments carefully.

Use this to create a "sandbox" for safe execution of non-trusted scripts.

If you only run your own scripts or plugins then you may not need this.

The code in this area is executed after each Lua script space is created
but before any of your scripts are done. This can be used to initialise things
(eg. load DLLs, load files, set up variables) or to disable things as shown below.

By setting a function name to nil you effectively make it unavailable.

You can remove some functions from a library rather than all of them, eg.

  os.execute = nil  -- no operating system calls
  os.remove = nil   -- no deleting files
  os.rename = nil   -- no renaming files

This script will automatically be replaced if you completely delete it from
the Global Preferences, and restart MUSHclient. To avoid this, leave a comment
in (if you don't want any further action taken).

--]]

-- Example sandbox --

function MakeSandbox ()

  local function ReportDisabled (pkg, func)
     return function ()
       error (string.format (
        "Function '%s.%s' disabled in Lua sandbox - see MUSHclient global preferences",
        pkg, func), 2)
       end -- function
  end -- ReportDisabled 

  package.loadlib = ReportDisabled  ("package", "loadlib") -- disable loadlib function
  package.loaders [3] = nil  -- disable DLL loader
  package.loaders [4] = nil  -- disable all-in-one loader

  for k, v in pairs (io) do
    if type (v) == "function" then
      io [k] = ReportDisabled ("io", k)
    end -- type is function
  end -- for

  local orig_os = os -- so we know names of disabled ones

  -- replace 'os' table with one containing only safe functions
  os = {
       date = os.date,
       time = os.time, 
       setlocale = os.setlocale,
       clock = os.clock, 
       difftime = os.difftime,
       }

  for k, v in pairs (orig_os) do
    if not os [k] and type (v) == "function" then
      os [k] = ReportDisabled ("os", k)
    end -- not still active
  end -- for

   if warn_if_not_trusted then
     ColourNote ("yellow", "black", 
                 "Lua sandbox created, some functions disabled.")
   end -- if warn_if_not_trusted

end -- end of function MakeSandbox


-- default is to sandbox everything --

-- To trust individual worlds or plugins, add them to the lists below.
                             
-- To find your current world ID, do this: /print (GetWorldID ())
-- Plugin IDs are mentioned near the start of every plugin.

-- You can limit the behaviour to specific worlds, or specific plugins
-- by doing something like this:

do

  -- World IDs of worlds we trust - replace with your world IDs
  --    (and remove comment from start of line)

  local trusted_worlds = {
 --    ["a4a1cc1801787ba88cd84f3a"] = true,  -- example world A
 --    ["cdc8552d1b251e449b874b9a"] = true,  -- example world B
 --    ["1ec5aac3265e472b97f0c103"] = true,  -- example world C
      }  -- end of trusted_worlds 

  -- Plugin IDs of plugins we trust - add your plugins to the table

  local trusted_plugins = {
     [""] = "",            -- trust main script (ie. if no plugin running)
     ["03ca99c4e98d2a3e6d655c7d"] = "Chat",  
     ["982581e59ab42844527eec80"] = "Random_Socials", 
     ["4a267cd69ba59b5ecefe42d8"] = "Installer_sumcheck",  
     ["83beba4e37b3d0e7f63cedbc"] = "Reconnecter",   
     }  -- end of trusted_plugins 


  -- check worlds 
  if not trust_all_worlds then                
    if not trusted_worlds [GetWorldID ()] then
       if warn_if_not_trusted  then
         ColourNote ("yellow", "black", "Untrusted world " .. WorldName () .. 
                     ", ID: " .. GetWorldID ())
       end -- if warn_if_not_trusted
       MakeSandbox ()
    end -- not trusted world or plugin 
  end -- not trusting all worlds

  -- check plugins - check name *and* plugin ID
  if not trust_all_plugins then
    if trusted_plugins [GetPluginID ()] ~= GetPluginName () then
       if warn_if_not_trusted  then
         ColourNote ("yellow", "black", "Untrusted plugin " .. GetPluginName () .. 
                     ", ID: " .. GetPluginID ())
       end -- if warn_if_not_trusted 
       MakeSandbox ()
    end -- not trusted world or plugin
  end -- if not trusting all plugins

end -- local block

-- warn if we can't load DLLs (checkbox might be unchecked)
if not package.loadlib and warn_if_not_trusted  then
   local by_this_plugin = ""
   if GetPluginID () ~= "" then
     by_this_plugin = " by this plugin"
   end -- this is a plugin
   ColourNote ("yellow", "black", 
               "Loading of DLLs" .. by_this_plugin .. " is disabled.")
end -- if
