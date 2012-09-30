-- wait.lua
-- ----------------------------------------------------------
-- 'wait' stuff - lets you build pauses into scripts.
-- See forum thread:
--   http://www.gammon.com.au/forum/?id=4957
-- ----------------------------------------------------------

--[[

Example of an alias 'send to script':


require "wait"

wait.make (function ()  --- coroutine below here

  repeat
    Send "cast heal"
    line, wildcards = 
       wait.regexp ("^(You heal .*|You lose your concentration)$")

  until string.find (line, "heal")

  -- wait a second for luck
  wait.time (1) 

end)  -- end of coroutine

--]]

require "check"

module (..., package.seeall)

-- ----------------------------------------------------------
-- table of outstanding threads that are waiting
-- ----------------------------------------------------------
local threads = {}

-- ----------------------------------------------------------
-- wait.timer_resume: called by a timer to resume a thread
-- ----------------------------------------------------------
function timer_resume (name)
  local thread = threads [name]
  if thread then
    threads [name] = nil
    local ok, err = coroutine.resume (thread)
    if not ok then
       ColourNote ("deeppink", "black", "Error raised in timer function (in wait module).")
       ColourNote ("darkorange", "black", debug.traceback (thread))
       error (err)
    end -- if
  end -- if
end -- function timer_resume 

-- ----------------------------------------------------------
-- wait.trigger_resume: called by a trigger to resume a thread
-- ----------------------------------------------------------
function trigger_resume (name, line, wildcards, styles)
  local thread = threads [name]
  if thread then
    threads [name] = nil
    local ok, err = coroutine.resume (thread, line, wildcards, styles)
    if not ok then
       ColourNote ("deeppink", "black", "Error raised in trigger function (in wait module)")
       ColourNote ("darkorange", "black", debug.traceback (thread))
       error (err)
    end -- if
  end -- if
end -- function trigger_resume 

-- ----------------------------------------------------------
-- convert x seconds to hours, minutes, seconds (for AddTimer)
-- ----------------------------------------------------------
local function convert_seconds (seconds)
  local hours = math.floor (seconds / 3600)
  seconds = seconds - (hours * 3600)
  local minutes = math.floor (seconds / 60)
  seconds = seconds - (minutes * 60)
  return hours, minutes, seconds
end -- function convert_seconds 

-- ----------------------------------------------------------
-- wait.time: we call this to wait in a script
-- ----------------------------------------------------------
function time (seconds)
  local id = "wait_timer_" .. GetUniqueNumber ()
  threads [id] = assert (coroutine.running (), "Must be in coroutine")

  local hours, minutes, seconds = convert_seconds (seconds)

  check (AddTimer (id, hours, minutes, seconds, "",
                  bit.bor (timer_flag.Enabled,
                           timer_flag.OneShot,
                           timer_flag.Temporary,
                           timer_flag.ActiveWhenClosed,
                           timer_flag.Replace), 
                   "wait.timer_resume"))

  return coroutine.yield ()
end -- function time

-- ----------------------------------------------------------
-- wait.regexp: we call this to wait for a trigger with a regexp
-- ----------------------------------------------------------
function regexp (regexp, timeout, flags)
  local id = "wait_trigger_" .. GetUniqueNumber ()
  threads [id] = assert (coroutine.running (), "Must be in coroutine")
            
  check (AddTriggerEx (id, regexp, 
            "-- added by wait.regexp",  
            bit.bor (flags or 0, -- user-supplied extra flags, like omit from output
                     trigger_flag.Enabled, 
                     trigger_flag.RegularExpression,
                     trigger_flag.Temporary,
                     trigger_flag.Replace,
                     trigger_flag.OneShot),
            custom_colour.NoChange, 
            0, "",  -- wildcard number, sound file name
            "wait.trigger_resume", 
            12, 100))  -- send to script (in case we have to delete the timer)
 
  -- if timeout specified, also add a timer
  if timeout and timeout > 0 then
    local hours, minutes, seconds = convert_seconds (timeout)

    -- if timer fires, it deletes this trigger
    check (AddTimer (id, hours, minutes, seconds, 
                   "DeleteTrigger ('" .. id .. "')",
                   bit.bor (timer_flag.Enabled,
                            timer_flag.OneShot,
                            timer_flag.Temporary,
                            timer_flag.ActiveWhenClosed,
                            timer_flag.Replace), 
                   "wait.timer_resume"))

    check (SetTimerOption (id, "send_to", "12"))  -- send to script

    -- if trigger fires, it should delete the timer we just added
    check (SetTriggerOption (id, "send", "DeleteTimer ('" .. id .. "')"))  

  end -- if having a timeout

  return coroutine.yield ()  -- return line, wildcards
end -- function regexp 

-- ----------------------------------------------------------
-- wait.match: we call this to wait for a trigger (not a regexp)
-- ----------------------------------------------------------
function match (match, timeout, flags)
  return regexp (MakeRegularExpression (match), timeout, flags)
end -- function match 

-- ----------------------------------------------------------
-- wait.make: makes a coroutine and resumes it
-- ----------------------------------------------------------
function make (f)
  assert (type (f) == "function", "wait.make requires a function")
  
  -- More friendly failure, suggested by Fiendish
  local errors = {}
  if GetOption ("enable_timers") ~= 1 then
    table.insert (errors, "TIMERS")
  end -- if timers disabled
  if GetOption ("enable_triggers") ~= 1 then
    table.insert (errors, "TRIGGERS")
  end  -- if triggers disabled
  if #errors ~= 0 then
    ColourNote("white", "red", 
               "One of your scripts (in '" .. 
               (GetPluginInfo(GetPluginID(), 1) or "World") ..
                "') just did something that requires " ..
                table.concat (errors, " and ") ..
                " to be enabled, but they aren't. " ..
                "Please check your configuration settings.")
    return nil, "Trigger/Timers not enabled"  -- bad return
  end  -- if have errors
  coroutine.wrap (f) () -- make coroutine, resume it
  return true  -- good return
end -- make
