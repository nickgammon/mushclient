-- stuff already localized
locale = "en" -- change to suit you

dofile (locale .. ".lua")

-- make copy
original = { 
    messages = messages, 
    formatted = formatted, 
    times = times, 
    headings = headings 
    }

messages, formatted, times, headings = nil 

-- from distribution
dofile ("Localize_template.lua")

-- make copy
distribution = { 
    messages = messages, 
    formatted = formatted, 
    times = times, 
    headings = headings 
    }

messages, formatted, times, headings = nil 

function compare_table (name)
 local count = 0
 local old = original [name]
 local new = distribution [name]

 print ("Processing table", name)
 print ""

 -- new message is in distribution, but not in already localized file
  for k, v in pairs (new) do
    if not old [k] then
      count = count + 1
      print (string.format ("  New message: %q", k))
    end -- if not there
  end -- for
  
  
  print ("Found ", count, " new messages")  
  print ""

  count = 0
  
 -- old message is in already localized file, but not in distribution
  for k, v in pairs (old) do
    if not new [k] then
      count = count + 1
      print (string.format ("  Deleted message: %q", k))
    end -- if not there
  end -- for
  
  print ("Found ", count, " deleted messages")  
  print ""
  
end -- compare_table

compare_table ("messages")
compare_table ("formatted")
compare_table ("times")
compare_table ("headings")
