--[[

Localization build utility.

Written by Nick Gammon - 11th June 2007

First run this in the MUSHclient source directory:

xgettext  -kTMessageBox -kTranslate -o mushclient_static.po *.cpp
xgettext  -kTFormat -o mushclient_formatted.po *.cpp

Then run this file - this outputs english.lua - which is the localization template.

--]]

local testing = false
local total = 0

fo = assert (io.open ("Localize_template.lua", "w"))

fo:write ("-- MUSHclient localization file\n\n")
fo:write (os.date ("-- Written: %A, %d %B %Y at %H:%M:%S\n\n"))

-- try to work out number of arguments (see sprintf format)
-- eg.  %+06.4f

function MakeArgumentList (id)

  local _, count = string.gsub (id, "%%[%-+ #0]*[%d.]*[lh]?[cdiouxXeEfgGs]", "")
  
  -- have to rescan in case I64 inside format string
  local _, count2 = string.gsub (id, "%%[%-+ #0]*[%d.]*I64[cdiouxXeEfgGs]", "")

  count = count + count2
  local t = {}
  
  if count > 26 then
    count = 26
  end -- if too many to make
  
  for i = 1, count do
    table.insert (t, string.char (string.byte 'a' + i - 1))
  end -- for
  
  return table.concat (t, ", ")
  
end -- function MakeArgumentList

function DoSection (filename, tablename, section_header, isfunction)

  count = 0
  
  fi = assert (io.open (filename, "r"))
  line = fi:read () -- priming read

  fo:write ("-- " .. section_header .. "\n\n")
  fo:write (tablename .. " = {\n")
  
  while line do
    id = string.match (line, '^msgid "(.*)"') 
    if id then

      total = total + 1
      count = count + 1
    
      -- msgid "xx" line might be followed by extra lines with just "xxx" in them.
      
      repeat
        line = fi:read ()  -- get next line
        extra = string.match (line, '^"(.*)"$')
        if extra then
          id = id .. extra
        end -- got an extra line
      until not extra
                      
      if id ~= "" then
        fo:write (string.format ('  ["%s"] =\n', id))  -- id will be the same
        
        if isfunction then
          if testing then
            testdata = string.format (
              'string.format ("(i18n) %s", %s)', id, MakeArgumentList (id))
          else
            testdata = '""'
          end -- if
           
          fo:write (string.format ([[
    function (%s)
     
      return %s
    end,  -- function
]]
                    ,
                    MakeArgumentList (id), testdata)) 
        else
          if testing then
            testdata = "(i18n) " .. id
          else
            testdata = ""
          end -- if
          
          fo:write (string.format ('    "%s",\n', testdata))
        end -- not function
        
      end -- if not empty string
    
    else
      comment = string.match (line, '^#: (.*)')
      if comment then
        fo:write ("\n-- " .. comment .. "\n")
      end -- comment line
      
    end  -- if found msgid line
    line = fi:read ()  -- get next line
  end  -- while reading each line
  
  fo:write ("  } -- end " .. tablename .. "\n\n")

  fi:close ()
  
  print (string.format ("Total of %i in section %s", count, tablename))
  
end -- DoSection

DoSection ("mushclient_static.po", "messages", "Static messages", false)
DoSection ("mushclient_formatted.po", "formatted", "Formatted messages", true)
DoSection ("mushclient_time.po", "times", "Date and time strings", false)
DoSection ("mushclient_heading.po", "headings", "Dialog headings", false)

fo:close ()

print (string.format ("\n** Grand total of %i written", total))
