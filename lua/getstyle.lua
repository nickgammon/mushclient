-- getstyle.lua
--

--[[

See forum thread:  http://www.gammon.com.au/forum/?id=7818

GetStyle:
  Finds a style run corresponding to a given column 

  Returns nil if style run not found (eg. column out of range)

  If style run found returns: 
     * the style table (see below)
     * the character at that column
     * the style run number (eg. style 3)

The style table should contain the following:

  t.text        --> text of that (entire) style run
  t.length      --> length of the (entire) style run
  t.textcolour  --> text colour (RGB number)
  t.backcolour  --> background colour (RGB number)
  t.style       --> style bits (1=bold, 2=underline, 4=italic)

--]]

function GetStyle (styleRuns, wantedColumn)
local currentColumn = 1

   -- check arguments
   assert (type (styleRuns) == "table", 
           "First argument to GetStyle must be table of style runs")

   assert (type (wantedColumn) == "number" and wantedColumn >= 1, 
           "Second argument to GetStyle must be column number to find")

   -- go through each style
   for item, style in ipairs (styleRuns) do
     local position = wantedColumn - currentColumn + 1  -- where letter is in style
     currentColumn = currentColumn + style.length       -- next style starts here
     if currentColumn > wantedColumn then  -- if we are within this style
        return style, string.sub (style.text, position, position), item  -- done
     end -- if found column
   end -- for each style

   -- if not found: result is nil

end -- function GetStyle 
