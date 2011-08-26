-- commas.lua
-- ----------------------------------------------------------
-- Rounding, duration, comma functions
-- See forum thread:
--  http://www.gammon.com.au/forum/?id=7805

--[[

This function rounds any number to the closest integer.
The "tricky" case is exactly half-way. 
That is, should 1.5 round to 1 or 2? How about -1.5?

This function rounds 1.5 "up" to 2, and -1.5 "down" to -2.

--]]

-- ----------------------------------------------------------


-- round "up" to absolute value, so we treat negative differently
--  that is, round (-1.5) will return -2
  
function round (x)
  if x >= 0 then
    return math.floor (x + 0.5)
  end  -- if positive

  return math.ceil (x - 0.5)
end -- function round

--[[

Duration

This function is designed to display a time interval in "short form". 
That is, rounded to the nearest major time interval. Some examples of intervals:


    * 3.6 days - displays "4 d"
    * 3.5 days - displays "4 d"
    * 3.4 days - displays "3 d"

    * 3.6 hours - displays "4 h"
    * 3.5 hours - displays "4 h"
    * 3.4 hours - displays "3 h"

    * 3.6 minutes - displays "4 m"
    * 3.5 minutes - displays "4 m"
    * 3.4 minutes - displays "3 m"

    * 59 seconds - displays "59 s"
    * 58 seconds - displays "58 s"
    * 57 seconds - displays "57 s" ... and so on to "0 s"


--]]

-- ----------------------------------------------------------

function convert_time (secs)

  -- handle negative numbers
  local sign = ""
  if secs < 0 then
    secs = math.abs (secs)
    sign = "-"
  end -- if negative seconds
  
  -- weeks
  if secs >= (60 * 60 * 24 * 6.5) then
    return sign .. round (secs / (60 * 60 * 24 * 7)) .. " w"
  end -- 6.5 or more days
  
  -- days
  if secs >= (60 * 60 * 23.5) then
    return sign .. round (secs / (60 * 60 * 24)) .. " d"
  end -- 23.5 or more hours
  
  -- hours
  if secs >= (60 * 59.5) then
   return sign .. round (secs / (60 * 60)) .. " h"
  end -- 59.5 or more minutes
  
  -- minutes
  if secs >= 59.5 then
   return sign .. round (secs / 60) .. " m"
  end -- 59.5 or more seconds
  
  -- seconds
  return sign .. round (secs) .. " s"    
end -- function convert_time 

--[[

Commas in numbers

This function adds commas to big numbers. 
For example 123456 becomes "123,456".

--]]

-- ----------------------------------------------------------

function commas (num)
  assert (type (num) == "number" or
          type (num) == "string")
  
  local result = ""

  -- split number into 3 parts, eg. -1234.545e22
  -- sign = + or -
  -- before = 1234
  -- after = .545e22

  local sign, before, after =
    string.match (tostring (num), "^([%+%-]?)(%d*)(%.?.*)$")

  -- pull out batches of 3 digits from the end, put a comma before them

  while string.len (before) > 3 do
    result = "," .. string.sub (before, -3, -1) .. result
    before = string.sub (before, 1, -4)  -- remove last 3 digits
  end -- while

  -- we want the original sign, any left-over digits, the comma part,
  -- and the stuff after the decimal point, if any
  return sign .. before .. result .. after

end -- function commas


-- trim leading and trailing spaces from a string
function trim (s)
  return (string.gsub (s, "^%s*(.-)%s*$", "%1"))
end -- trim


--[[

Shuffle a table

-- see: http://en.wikipedia.org/wiki/Fisher-Yates_shuffle

Example:

cards = { "Ace", "King", "Queen", "Jack", 10, 9, 8, 7, 6, 5, 4, 3, 2 }

shuffle (cards)

--]]

function shuffle(t)
  local n = #t
 
  while n >= 2 do
    -- n is now the last pertinent index
    local k = math.random(n) -- 1 <= k <= n
    -- Quick swap
    t[n], t[k] = t[k], t[n]
    n = n - 1
  end
 
  return t
end


--[[

Directory scanner.

Calls function "f" for every file found in a path starting at "path".
Recurses to handle nested directories. Function "f" is called with two
arguments: (full) filename, statistics

See utils.readdir for the exact names of the entries supplied for each file.

See: http://www.gammon.com.au/forum/?id=9906


Example:


plugins = {}

-- this function is called for every found file
function load_file (name, stats)
 
  if stats.size > 0 and
     string.match (name:lower (), "%.xml$") and
     not stats.hidden then
       table.insert (plugins,  name)
  end -- if
  
end -- load_file

-- Scan plugins folder, passing each file found to "load_file" function.

scan_dir (GetInfo (60), load_file)

--]]


function scan_dir (path, f)

  -- find all files in that directory
  local t = assert (utils.readdir (path .. "\\*"))

  for k, v in pairs (t) do
      
    -- recurse to process subdirectory
    if v.directory then

      if k ~= "." and k ~= ".." then
        scan_dir (path .. "\\" .. k, f)
      end -- not current or owner directory

    else -- call supplied function
      f (path .. "\\" .. k, v)  
    end -- if 
 
  end -- for

end -- scan_dir 
