
-- make copy of original tables
orig_messages = messages
orig_formatted = formatted
orig_times = times
orig_headings = headings

-- empty them out so __index is triggered
-- save original tables so we can look them up eventually

messages  = { _orig = orig_messages }
formatted = { _orig = orig_formatted }
times     = { _orig = orig_times }
headings  = { _orig = orig_headings }

counts = {} -- keep counts here

-- metatable for messages, titles, headings
mt_static = {
  -- called to access an entry
  __index=
    function (t, name)
      local s = rawget (t._orig, name)
      if s == nil or #s == 0 then
        counts [name] = (counts [name] or 0) + 1
      end -- not translated yet
      return s
    end;
  }
  
-- metatable for formatted messages
mt_formatted = {
  -- called to access an entry
  __index=
    function (t, name)
      local f = rawget (t._orig, name)
      -- no function? not translated then
      if f == nil then
        counts [name] = (counts [name] or 0) + 1
        return nil
      end
      assert (type (f) == "function")
    
      -- return a function, that will count if the original function
      -- returns an empty string
      return function (...)
         local s = f (...)  -- call original function
         if type (s) ~= "string" or #s == 0 then
           counts [name] = (counts [name] or 0) + 1
         end -- not translated
         return s  -- return translated value
         end -- function 
     end;
  }
  
-- apply the metatables
setmetatable (messages,   mt_static)
setmetatable (times,      mt_static)
setmetatable (headings,   mt_static)
setmetatable (formatted,  mt_formatted)

-- the user will call world.TranslateDebug to invoke this
function Debug ()
   
  -- for sorting
  local t = {}
  
  -- build into table which can be sorted
  for k, v in pairs (counts) do
    table.insert (t, k)
  end -- for

  -- clear out notepad, make heading
  utils.appendtonotepad ("translation", "Translation counts\n\n", true)

  -- sort into descending order
  table.sort (t, function (a, b)
                  return counts [a] > counts [b]
                 end)
                    
  -- display results
  for k, v in ipairs (t) do
    utils.appendtonotepad ("translation", string.format ("%4i: %q \n", counts [v], v))
  end -- for
  
end -- Debug


