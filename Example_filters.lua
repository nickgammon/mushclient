-- ---------------------------------------------------------------
--  Example Trigger filters (paste into "filter by" script box
--    in the trigger list).
-- ---------------------------------------------------------------


function send_to_script (name, trigger)
  return trigger.send_to == sendto.script and 
         trigger.enabled
end -- send_to_script 

function enabled (name, trigger)
  return trigger.enabled
end -- enabled 

function disabled (name, trigger)
  return not trigger.enabled
end -- disabled 

function keep_evaluating (name, trigger)
  return trigger.keep_evaluating
end -- keep_evaluating 

function badscript (name, trigger)
  return not trigger.script_valid and
             trigger.script ~= ""
end -- badscript 

function temporary (name, trigger)
  return trigger.temporary
end -- temporary 

function matched (name, trigger)
  return trigger.times_matched > 0
end -- matched 

function unmatched (name, trigger)
  return trigger.times_matched == 0
end -- unmatched 

-- if they cancel, show everything
function everything (name, trigger) 
  return true
end -- everything 

-- choose which function to use
result = utils.listbox ("Choose type of filtering", "Triggers",
           { 
           send_to_script   = "Send to script and enabled",
           enabled          = "Enabled items",
           disabled         = "Disabled items",
           badscript        = "Script name not found",
           keep_evaluating  = "Keep evaluating",
           temporary        = "Temporary triggers",
           matched          = "Ones that matched something",
           unmatched        = "Ones that never matched",
           },
           "badscript") -- default

-- use that function
filter = _G [result] or everything



-- ---------------------------------------------------------------
--  Example Alias filters (paste into "filter by" script box
--    in the alias list).
-- ---------------------------------------------------------------

function send_to_script (name, alias)
  return alias.send_to == sendto.script and 
         alias.enabled
end -- send_to_script 

function enabled (name, alias)
  return alias.enabled
end -- enabled 

function disabled (name, alias)
  return not alias.enabled
end -- disabled 

function keep_evaluating (name, alias)
  return alias.keep_evaluating
end -- keep_evaluating 

function badscript (name, alias)
  return not alias.script_valid and
             alias.script ~= ""
end -- badscript 

function temporary (name, alias)
  return alias.temporary
end -- temporary 

function matched (name, alias)
  return alias.times_matched > 0
end -- matched 

function unmatched (name, alias)
  return alias.times_matched == 0
end -- unmatched 

-- if they cancel, show everything
function everything (name, alias) 
  return true
end -- everything 

-- choose which function to use
result = utils.listbox ("Choose type of filtering", "Aliases",
           { 
           send_to_script   = "Send to script and enabled",
           enabled          = "Enabled items",
           disabled         = "Disabled items",
           badscript        = "Script name not found",
           keep_evaluating  = "Keep evaluating",
           temporary        = "Temporary aliases",
           matched          = "Ones that matched something",
           unmatched        = "Ones that never matched",
           },
           "badscript") -- default

-- use that function
filter = _G [result] or everything


-- ---------------------------------------------------------------
--  Example Timer filters (paste into "filter by" script box
--    in the timer list).
-- ---------------------------------------------------------------


function send_to_script (name, timer)
  return timer.send_to == sendto.script and 
         timer.enabled
end -- send_to_script 

function enabled (name, timer)
  return timer.enabled
end -- enabled 

function disabled (name, timer)
  return not timer.enabled
end -- disabled 

function one_shot (name, timer)
  return timer.one_shot
end -- one_shot 

function badscript (name, timer)
  return not timer.script_valid and
             timer.script ~= ""
end -- badscript 

function temporary (name, timer)
  return timer.temporary
end -- temporary 

function fired (name, timer)
  return timer.times_fired > 0
end -- fired 

function not_fired (name, timer)
  return timer.times_fired == 0
end -- not_fired 

-- if they cancel, show everything
function everything (name, timer) 
  return true
end -- everything 

-- choose which function to use
result = utils.listbox ("Choose type of filtering", "Timers",
           { 
           send_to_script = "Send to script and enabled",
           enabled        = "Enabled items",
           disabled       = "Disabled items",
           badscript      = "Script name not found",
           one_shot       = "One-shot timers",
           temporary      = "Temporary timers",
           fired          = "Ones that fired",
           not_fired      = "Ones that never fired",
           },
           "badscript") -- default

-- use that function
filter = _G [result] or everything

