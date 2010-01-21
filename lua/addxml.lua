-- addxml.lua

-- lets you add triggers, aliases, timers, macros using XML code
--
-- See: http://www.gammon.com.au/forum/?id=7123

--[[

This lets you add items by simply setting up a table of their attributes and then adding it.

Exposed functions are:

   addxml.trigger  -- add a trigger
   addxml.alias    -- add an alias
   addxml.timer    -- add a timer
   addxml.macro    -- add a macro
   addxml.save     -- convert one of the above back into a table
   
Example of adding a trigger:

require "addxml"
addxml.trigger {  match = "swordfish", 
                regexp = true,
                ['repeat'] = true,   -- repeat is lua keyword
                send = "hi there",
                sequence = 50,
                enabled = true,
                name = "boris",
              }

Example of converting a trigger back into a table:
              
require "tprint"
require "addxml"
tprint (addxml.save ("trigger", "boris"))

--]]


module (..., package.seeall)

local html_replacements = { 
   ["<"] = "&lt;",
   [">"] = "&gt;",
   ["&"] = "&amp;",
   ['"'] = "&quot;",
   }

-- fix text so that < > & and double-quote are escaped
local function fixhtml (s)

  return (string.gsub (tostring (s), '[<>&"]', 
    function (str)
      return html_replacements [str] or str
    end ))

end -- fixhtml


local function GeneralAdd (t, which, plural)
 
  assert (type (t) == "table", "Table must be supplied to add a " .. which)

  local k, v
  local xml = {}
  
  local send = fixhtml (t.send or "")  -- send is done differently
  t.send = nil
  
  -- turn into XML options  
  for k, v in pairs (t) do
  
    -- fix true/false to y/n
    if v == true then
      v = "y"
    elseif v == false then
      v = "n"
    end -- if true or false
    
    table.insert (xml, k .. '="' .. fixhtml (v) .. '"')
  end -- for loop
      
  assert (ImportXML (string.format (
          "<%s><%s %s ><send>%s</send></%s></%s>",
             plural,   -- eg. triggers
             which,    -- eg. trigger
             table.concat (xml, "\n"),  -- eg. match="nick"
             send,     -- eg. "go north"
             which,    -- eg. trigger
             plural)   -- eg. triggers
         ) == 1, "Import of " .. which .. " failed") 
  
end -- GeneralAdd 

function trigger (t)
  GeneralAdd (t, "trigger", "triggers")
  -- force script entry-point resolution
  if t.name and t.script then
    SetTriggerOption (t.name, "script", t.script)
  end -- if trigger has a name, and a script name
end -- addxml.trigger

function alias (t)
  GeneralAdd (t, "alias", "aliases")
  -- force script entry-point resolution
  if t.name and t.script then
    SetAliasOption (t.name, "script", t.script)
  end -- if alias has a name, and a script name
end -- addxml.alias 

function timer (t)
  GeneralAdd (t, "timer", "timers")
  -- force script entry-point resolution
  if t.name and t.script then
    SetTimerOption (t.name, "script", t.script)
  end -- if timer has a name, and a script name
end -- addxml.timer 

function macro (t)
  GeneralAdd (t, "macro", "macros")
end -- addxml.macro

function save (which, name)

  local typeconversion = 
     {
     trigger = 0,
     alias = 1,
     timer = 2,
     macro = 3,
 --    variable = 4,
 --   keypad = 5 
     }
     
  local itemtype = assert (typeconversion [which], "Unknown type: " .. which)
  
  local xml = ExportXML (itemtype, name)
  
  -- if not found returns empty string
  assert (xml ~= "", "Can't find " .. which .. ": " .. name)
  
  -- parse back into table entries
  local xmlnodes = assert (utils.xmlread (xml), "Bad XML")
  
  -- all attributes should be a couple of levels down                    
  local result = xmlnodes.nodes [1].nodes [1].attributes
                   
  -- find "send" text
  
  -- another level?
  if xmlnodes.nodes [1].nodes [1].nodes then
    if xmlnodes.nodes [1].nodes [1].nodes [1].name == "send" then
       result.send = xmlnodes.nodes [1].nodes [1].nodes [1].content
    end -- have a "send" field
  end -- have a child node
  
  return result
end -- addxml.save
