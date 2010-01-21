-- ---------------------------------------------------------------
--  Example Trigger filters (paste into "filter by" script box
--    in the trigger list).
-- ---------------------------------------------------------------


function send_to_script (s)
  return GetTriggerInfo (s, 15) == 12 and -- send to script
         GetTriggerInfo (s, 8)            -- enabled
end -- send_to_script 

function enabled (s)
  return GetTriggerInfo (s, 8)  -- enabled
end -- enabled 

function disabled (s)
  return not GetTriggerInfo (s, 8)  -- not enabled
end -- disabled 

function keep_evaluating (s)
  return GetTriggerInfo (s, 7)  -- keep-evaluating flag
end -- keep_evaluating 

function badscript (s)
  return not GetTriggerInfo (s, 34) and  -- script not found
             GetTriggerInfo (s, 4) ~= "" -- but name given
end -- badscript 

function temporary (s)
  return GetTriggerInfo (s, 23)  -- temporary
end -- temporary 

function matched (s)
  return GetTriggerInfo (s, 21) > 0 -- match count
end -- matched 

function unmatched (s)
  return GetTriggerInfo (s, 21) == 0 -- match count
end -- unmatched 

-- if they cancel, show everything
function everything (s) 
  return true
end -- everything 

-- choose which function to use
result = utils.listbox ("Choose type of filtering", "Triggers",
           { 
           send_to_script = "Send to script and enabled",
           enabled = "Enabled items",
           disabled = "Disabled items",
           badscript = "Script name not found",
           keep_evaluating = "Keep evaluating",
           temporary = "Temporary triggers",
           matched = "Ones that matched something",
           unmatched = "Ones that never matched",
           },
           "badscript") -- default

-- use that function
filter = _G [result] or everything



-- ---------------------------------------------------------------
--  Example Alias filters (paste into "filter by" script box
--    in the alias list).
-- ---------------------------------------------------------------



function send_to_script (s)
  return GetAliasInfo (s, 18) == 12 and -- send to script
         GetAliasInfo (s, 6)            -- enabled
end -- send_to_script 

function enabled (s)
  return GetAliasInfo (s, 6)  -- enabled
end -- enabled 

function disabled (s)
  return not GetAliasInfo (s, 6)  -- not enabled
end -- disabled 

function keep_evaluating (s)
  return GetAliasInfo (s, 19)  -- keep-evaluating flag
end -- keep_evaluating 

function badscript (s)
  return not GetAliasInfo (s, 27) and  -- script not found
             GetAliasInfo (s, 3) ~= "" -- but name given
end -- badscript 

function temporary (s)
  return GetAliasInfo (s, 14)  -- temporary
end -- temporary 

function matched (s)
  return GetAliasInfo (s, 11) > 0 -- match count
end -- matched 

function unmatched (s)
  return GetAliasInfo (s, 11) == 0 -- match count
end -- unmatched 

-- if they cancel, show everything
function everything (s) 
  return true
end -- everything 

-- choose which function to use
result = utils.listbox ("Choose type of filtering", "Aliases",
           { 
           send_to_script = "Send to script and enabled",
           enabled = "Enabled items",
           disabled = "Disabled items",
           badscript = "Script name not found",
           keep_evaluating = "Keep evaluating",
           temporary = "Temporary aliases",
           matched = "Ones that matched something",
           unmatched = "Ones that never matched",
           },
           "badscript") -- default

-- use that function
filter = _G [result] or everything


-- ---------------------------------------------------------------
--  Example Timer filters (paste into "filter by" script box
--    in the timer list).
-- ---------------------------------------------------------------


function send_to_script (s)
  return GetTimerInfo (s, 20) == 12 and -- send to script
         GetTimerInfo (s, 6)            -- enabled
end -- send_to_script 

function enabled (s)
  return GetTimerInfo (s, 6)  -- enabled
end -- enabled 

function disabled (s)
  return not GetTimerInfo (s, 6)  -- not enabled
end -- disabled 

function one_shot (s)
  return GetTimerInfo (s, 7)  -- one_shot flag
end -- one_shot 

function badscript (s)
  return not GetTimerInfo (s, 26) and  -- script not found
             GetTimerInfo (s, 5) ~= "" -- but name given
end -- badscript 

function temporary (s)
  return GetTimerInfo (s, 14)  -- temporary
end -- temporary 

function matched (s)
  return GetTimerInfo (s, 10) > 0 -- match count
end -- matched 

function unmatched (s)
  return GetTimerInfo (s, 10) == 0 -- match count
end -- unmatched 

-- if they cancel, show everything
function everything (s) 
  return true
end -- everything 

-- choose which function to use
result = utils.listbox ("Choose type of filtering", "Timers",
           { 
           send_to_script = "Send to script and enabled",
           enabled = "Enabled items",
           disabled = "Disabled items",
           badscript = "Script name not found",
           one_shot = "One-shot timers",
           temporary = "Temporary timers",
           matched = "Ones that fired",
           unmatched = "Ones that never fired",
           },
           "badscript") -- default

-- use that function
filter = _G [result] or everything

