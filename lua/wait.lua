-- wait.lua
-- ----------------------------------------------------------
-- 'wait' stuff - lets you build pauses into scripts.
-- See forum thread:
--   http://www.gammon.com.au/forum/?id=4957
--
--Modified by simon
--Date 2013.12.10
--Email:hapopen@gmail.com
--Fix: 2014.1.20
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
function regexp (regexp, timeout, flags, multi, multi_lines)
  if type(regexp) == "table" then
    regexp = "(" .. table.concat (regexp, "|") .. ")"
  end
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
  if multi then
    SetTriggerOption (id, "multi_line", multi)
    if multi_lines then
      SetTriggerOption (id, "lines_to_match", multi_lines)
    end
  end
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


------------------------------------------
-- Multi-function timer expires trigger call back
--
-- Returns the newly created object
------------------------------------------
function timer_mulresume (name)
  DeleteTriggerGroup(name)
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
end -- function timer_mulresume

------------------------------------------
-- Multi-trigger function call back
--
-- return the new object
------------------------------------------
function trigger_mulresume (name, line, wildcards, styles)
  local gp=GetTriggerOption(name,"group")
  DeleteTriggerGroup(gp)
  DeleteTimer(gp)
  local thread = threads [gp]
  if thread then
    threads [gp] = nil
    local ok, err = coroutine.resume (thread, line, wildcards, styles)
    if not ok then
       ColourNote ("deeppink", "black", "Error raised in trigger function (in wait module)")
       ColourNote ("darkorange", "black", debug.traceback (thread))
       error (err)
    end -- if
  end -- if
end -- function trigger_mulresume

------------------------------------------
-- This function waits for a trigger multiple regular return.
-- After a trigger to return any other triggers will also fail.
--  @regexps   List of regular expressions.
--           {Regular expressions, whether multi-line trigger, trigger rows}
--  @timeout   Timeout in seconds
--  @flags     user-supplied extra flags, like omit from output
--
--Returns the newly created object
------------------------------------------
function mulregexp (regexps, timeout, flags)
  local gp = "wait_group_" .. GetUniqueNumber ()
  threads [gp] = assert (coroutine.running (), "Must be in coroutine")
  local id
  for k,v in pairs(regexps) do
      id = "wait_trigger_" .. GetUniqueNumber ()
      check (AddTriggerEx (id, v[1],
      "-- added by wait.mulregexp",
      bit.bor (flags or 0, -- user-supplied extra flags, like omit from output
         trigger_flag.Enabled,
         trigger_flag.RegularExpression,
         trigger_flag.Temporary,
         trigger_flag.Replace,
         trigger_flag.OneShot),
      custom_colour.NoChange,
      0, "",  -- wildcard number, sound file name
      "wait.trigger_mulresume",
      12, 100))  -- send to script (in case we have to delete the timer)
    SetTriggerOption (id, "group", gp)
    if v [2] then
      SetTriggerOption (id, "multi_line", true)
      if v[3] then
        SetTriggerOption (id, "lines_to_match", v[3])
      end
    end
  end

  -- if timeout specified, also add a timer
  if timeout and timeout > 0 then
    local hours, minutes, seconds = convert_seconds (timeout)

    -- if timer fires, it deletes this trigger
    check (AddTimer (gp, hours, minutes, seconds,
       "-- added by wait.mulregexp",
                   bit.bor (timer_flag.Enabled,
                            timer_flag.OneShot,
                            timer_flag.Temporary,
                            timer_flag.ActiveWhenClosed,
                            timer_flag.Replace),
                   "wait.timer_mulresume"))

    check (SetTimerOption (gp, "send_to", "12"))  -- send to script

  end -- if having a timeout

  return coroutine.yield ()  -- return line, wildcards
end
------------------------------------------
--Restore a specified process
--  @sign  Pause logo
--  @text  Returns a string to suspend location
--
--Returns the newly created object
------------------------------------------
function wake (sign,text)
  local name="wait_pause_"..sign
  local thread = threads [name]
  if thread then
    threads [name] = nil
    local ok, err = coroutine.resume (thread,text)
    if not ok then
       ColourNote ("deeppink", "black", "Error raised in timer function (in wait module).")
       ColourNote ("darkorange", "black", debug.traceback (thread))
       error (err)
    end -- if
  end -- if
end
------------------------------------------
-- Suspend a process until wake is called
--@sign  pause location identification wordmark
--
--return the new object
------------------------------------------
function pause (sign)
  local id = "wait_pause_"..sign
  threads [id] = assert (coroutine.running (), "Must be in coroutine")
  return coroutine.yield ()  -- return line, wildcards
end

subwait={}
------------------------------------------
-- Delete the array elements specified in the table, delete only one
--@arr     table
--@element element
--
--returns the new table
------------------------------------------
function table_del(arr, element)
   if arr==nil then
      return nil
   end
   if #arr==0 then
      return {}
   end
   for k, value in pairs(arr) do
      if value == element then
         table.remove(arr, k)
         return arr
      end
   end
   return arr
end
------------------------------------------
--Start a new thread object and returns a new thread
--
--Returns the newly created object
------------------------------------------
function subwait:new ()
    local o = {list={["timer"]={},["trigger"]={},["group"]={}}}
    setmetatable(o, self)
    self.__index = self
    return o
end
------------------------------------------
--Close thread object
--
--return  No
------------------------------------------
function subwait:close ()
    for key,value in pairs(self.list["timer"]) do
      DeleteTimer(value)
      threads[value]=nil
    end
    self.list["timer"]={}
    for key,value in pairs(self.list["trigger"]) do
      DeleteTrigger(value)
      DeleteTimer(value)
      threads[value]=nil
    end
    self.list["trigger"]={}
    for key,value in pairs(self.list["group"]) do
      DeleteTriggerGroup(value)
      DeleteTimer(value)
      threads[value]=nil
    end
    self.list["group"]={}
end
------------------------------------------
--Child thread waits
--@seconds
--
--return  No
------------------------------------------
function subwait:time (seconds)
  local id = "subwait_timer_" .. GetUniqueNumber ()
  threads [id] = assert (coroutine.running (), "Must be in coroutine")

  local hours, minutes, seconds = convert_seconds (seconds)

  check (AddTimer (id, hours, minutes, seconds, "",
                  bit.bor (timer_flag.Enabled,
                           timer_flag.OneShot,
                           timer_flag.Temporary,
                           timer_flag.ActiveWhenClosed,
                           timer_flag.Replace),
                   "wait.timer_resume"))
  table.insert(self.list["timer"],id)
  --return coroutine.yield ()
  coroutine.yield ()
  self.list["timer"]=table_del(self.list["timer"],id)
  return

end
------------------------------------------
--Child thread waits for a trigger
--
--return  Text data triggered
------------------------------------------
function subwait:regexp (regexp, timeout, flags, multi, multi_lines)
  if type(regexp) == "table" then
    regexp = "(" .. table.concat (regexp, "|") .. ")"
  end
  local id = "subwait_trigger_" .. GetUniqueNumber ()
  threads [id] = assert (coroutine.running (), "Must be in coroutine")

  check (AddTriggerEx (id, regexp,
            "-- added by subwait.regexp",
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
  if multi then
    SetTriggerOption (id, "multi_line", true)
    if multi_lines then
      SetTriggerOption (id, "lines_to_match", multi_lines)
    end
  end
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

  --return coroutine.yield ()  -- return line, wildcards
  table.insert(self.list["trigger"],id)
  local line, wildcards, styles
  line, wildcards, styles=coroutine.yield ()
  self.list["trigger"]=table_del(self.list["trigger"],id)
  return line, wildcards, styles
end
------------------------------------------
-- This function waits for multiple regular trigger returned.
-- After a trigger to return any other triggers will also fail.
--  @regexps    list of regular expressions.
--            {Regular expressions, whether multi-line trigger, trigger rows}
--  @timeout   timeout in seconds
--  @flags     user-supplied extra flags, like omit from output
--
--return  return the new object
------------------------------------------
function subwait:mulregexp (regexps, timeout, flags)
  local gp = "wait_group_" .. GetUniqueNumber ()
  threads [gp] = assert (coroutine.running (), "Must be in coroutine")
  local id
  for k,v in pairs(regexps) do
      id = "wait_trigger_" .. GetUniqueNumber ()
      check (AddTriggerEx (id, v[1],
    "-- added by wait.mulregexp",
    bit.bor (flags or 0, -- user-supplied extra flags, like omit from output
       trigger_flag.Enabled,
       trigger_flag.RegularExpression,
       trigger_flag.Temporary,
       trigger_flag.Replace,
       trigger_flag.OneShot),
    custom_colour.NoChange,
    0, "",  -- wildcard number, sound file name
    "wait.trigger_mulresume",
    12, 100))  -- send to script (in case we have to delete the timer)
    SetTriggerOption (id, "group", gp)
    if  v[2] then
      SetTriggerOption (id, "multi_line", true)
      if v[3] then
        SetTriggerOption (id, "lines_to_match", v[3])
      end
    end
  end

  -- if timeout specified, also add a timer
  if timeout and timeout > 0 then
    local hours, minutes, seconds = convert_seconds (timeout)

    -- if timer fires, it deletes this trigger
    check (AddTimer (gp, hours, minutes, seconds,
       "-- added by wait.mulregexp",
                   bit.bor (timer_flag.Enabled,
                            timer_flag.OneShot,
                            timer_flag.Temporary,
                            timer_flag.ActiveWhenClosed,
                            timer_flag.Replace),
                   "wait.timer_mulresume"))

    check (SetTimerOption (gp, "send_to", "12"))  -- send to script

  end -- if having a timeout

  --return coroutine.yield ()  -- return line, wildcards
  table.insert(self.list["group"],gp)
  local line, wildcards, styles
  line, wildcards, styles=coroutine.yield ()
  self.list["group"]=table_del(self.list["group"],gp)
  return line, wildcards, styles
end
