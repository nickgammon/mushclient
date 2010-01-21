-- Example of coding script routines for MUSHclient in Lua

-- --------------------------------------------
-- Example trigger routine that just shows what was passed to it
-- --------------------------------------------

function ExampleTrigger (thename, theoutput, wildcards, line)

  require "tprint"

  Note ("Trigger " .. thename .. " fired.")
  Note ("Matching line was: " .. theoutput)
  Note ("Wildcards ...")
  tprint (wildcards)
  Note ("Line with style runs ...")
  tprint (line)

end -- of ExampleTrigger

-- ----------------------------------------------------------
-- Example showing iterating through all triggers
-- ----------------------------------------------------------

function showtriggers ()
  local _, v
  for _, v in ipairs (GetTriggerList ()) do 
    Note (v) 
  end
end  -- of showtriggers

-- -----------------------------------------------
-- Example showing iterating through all variables
-- ------------------------------------------------

function showvariables ()
  table.foreach (GetVariableList(), print)
end 	-- of showvariables 

-- ----------------------------------------------------------
-- Example showing iterating through all aliases with labels
-- ----------------------------------------------------------

function showaliases ()
  local _, v
  for _, v in ipairs (GetAliasList ()) do 
    Note (v) 
  end
end  -- of showaliases


-- ---------------------------------------------------------
-- Example showing running a script on world open
-- ---------------------------------------------------------
function OnWorldOpen ()
  Note ("---------- World Open ------------")
end	-- of OnWorldOpen 

-- ---------------------------------------------------------
-- Example showing running a script on world close
-- ---------------------------------------------------------
function OnWorldClose ()
  Note ("---------- World Close ------------")
end	-- of OnWorldClose 

-- ---------------------------------------------------------
-- Example showing running a script on world connect
-- ---------------------------------------------------------
function OnWorldConnect ()
  Note ("---------- World Connect ------------")
end	-- of OnWorldConnect 

-- ---------------------------------------------------------
-- Example showing running a script on world disconnect
-- ---------------------------------------------------------
function OnWorldDisconnect ()
  Note ("---------- World Disconnect  ------------")
end	-- of OnWorldDisconnect 

-- ---------------------------------------------------------
-- Example showing running a script on an alias
--
--  This script is designed to be called by an alias: ^teleport(.*)$
--
--  This alias SHOULD have "regular expression" checked.
--
--  It is for teleporting (going to) a room by number
--
--  The room is entered by name and looked up in the variables
--  list.
-- ---------------------------------------------------------
function OnTeleport (name, output, wildcards)

  local sDestination
  local sHelp = ""
  local iRoom
  
  sDestination = Trim (wildcards [1])
  
  -- if nothing entered echo possible destinations
  if sDestination == "" then
  
    Note ("-------- TELEPORT destinations ----------")
  
    local k, v
    for k, v in pairs (GetVariableList ()) do
      if string.sub (k, 1, 9) == "teleport_" then
        if sHelp ~= "" then
           sHelp = sHelp .. ", "
        end
        sHelp = sHelp .. string.sub (k, 10)
      end
    end
  
    -- if no destinations found, tell them
    if sHelp == "" then
      sHelp = "<no rooms in teleport list>"
    end
    Note (sHelp)
    return
    end    -- no destination supplied
  
  -- get contents of the destination variable 
  
  iRoom = world.GetVariable ("teleport_" .. string.lower (sDestination))
  
  -- if not found, or invalid name, that isn't in the list
  if iRoom == nil then
    Note ("******** Destination " .. sDestination .. " unknown *********")
    return
  end
  
  Note ("------> Teleporting to " .. sDestination)
  Send ("@teleport #" .. iRoom)

end	-- end of OnTeleport 


-- ---------------------------------------------------------
-- Example showing running a script on an alias
--
--  This script is designed to be called by an alias:  ^add_teleport(|\s*(\w*)\s*(\d*))$
--
--  This alias SHOULD have "regular expression" checked.
--
--  It is for adding a room to the list of rooms to teleport to (by
--  the earlier script).
--
--  eg.  ADD_TELEPORT dungeon 1234
--
-- ---------------------------------------------------------
function OnAddTeleport (name, output, wildcards)

  local sDestination
  local iRoom
  local iStatus
  
  sDestination = Trim (wildcards [2])
  
  -- if nothing entered tell them command syntax
  if sDestination == "" then
    Note ("Syntax: add_teleport name dbref")
    Note ("    eg. add_teleport LandingBay 4421")
    return
    end
  
  -- room to go to
  iRoom = wildcards [3]
  
  if not tonumber (iRoom) then
    Note ("Room number must be numeric, you entered " .. iRoom)
    return
  end
  
  -- add room and destination location to variable list
  iStatus = world.SetVariable ("teleport_" .. string.lower (sDestination), iRoom)
  
  if iStatus ~= 0 then
    Note ("Room name must be alphabetic, you entered: " + sDestination)
    return
  end
  
  Note ("Teleport location " .. sDestination .. "(#" 
             .. iRoom .. ") added to teleport list")

end  -- of OnAddTeleport 

-- ------------------------------------------
-- Example showing a script called by a timer
-- -------------------------------------------
function OnTimer (strTimerName)
  Note ("Timer has fired!")
end	-- of OnTimer 


-- --------------------------------------------
-- Example showing a script called by a trigger
--  Should be connected to a trigger matching on: <*hp *m *mv>*
--   (the above example will work for SMAUG default prompts (eg. <100hp 10m 40mv>)
--    it may need to be changed depending on the MUD prompt format).
-- --------------------------------------------
function OnStats (name, trig_line, wildcards)

  local iHP = wildcards [1]
  local iMana = wildcards [2]
  local iMV = wildcards [3]
  
  Note ("Your HP are " .. iHP)
  Note ("Your Mana is " .. iMana)
  Note ("Your movement points are " .. iMV)

end	-- of OnStats 


-- --------------------------------------------
-- Subroutine to be called to repeat a command.
--
--  Call from the alias: ^#(\d+)\s+(.+)$
--  Regular Expression: checked
--
--  Example of use:  #10 give sword to Trispis
--  This would send "give sword to Trispis" 10 times
-- --------------------------------------------
function OnRepeat (name, line, wildcards)

local count = wildcards [1]

  if not tonumber (count) then
    Note ("Repeat count must be numeric, you entered ", count)
    return
  end

  local i
  for i = 1, count do
    Send (wildcards [2])
  end
      
end  -- of OnRepeat 

-- --------------------------------------------
-- Example showing iterating through all worlds
-- --------------------------------------------

function showworlds ()
  local _, v

  for _, v in ipairs (GetWorldList ()) do 
    Note (v) 
  end
  
end	-- of showworlds 

-- --------------------------------------------------
-- Example showing sending a message to another world
-- --------------------------------------------------

function SendToWorld (name, message)

local otherworld

  otherworld = GetWorld (name)

  if otherworld == nil then
    Note ("World " .. name .. " is not open")
    return
  end

  Send (otherworld, message)

  -- alternative syntax:   otherworld:Send (message)

end -- of SendToWorld


Note ("Lua scripting enabled - script file processed")

