-- mw.lua

-- Helper functions for miniwindows
--

-- Author: Nick Gammon - 8th September 2008

--[[

Exposed functions are:

   mw.colourtext        - show a string with imbedded colour codes
   mw.colour_conversion - table with colour codes in it - add more if you want
   mw.strip_colours     - remove colour codes from a string
   mw.popup             - make a popup window
   
Example of making a popup window:

  require "mw"
  
  
  -- SET UP FOR POPUP WINDOWS - define colours, add fonts, make window id
  -- (DO THIS ONCE ONLY, eg. in OnPluginInstall)
  
  -- our window frame/background colours
  border_colour = 0xCCD148
  background_colour = 0x222222
  
  -- a unique ID
  infowin = GetPluginID () .. ":info"

  -- font IDs
  font_id = "popup_font"  
  heading_font_id = "popup_heading_font"  

  font_size = 8

  -- use 8 pt Dina or 10 pt Courier
  local fonts = utils.getfontfamilies ()

  -- choose a font that exists
  
  if fonts.Dina then
    font_name = "Dina"    
  elseif fonts ["Lucida Sans Unicode"] then
    font_name = "Lucida Sans Unicode"
  else
    font_size = 10
    font_name = "Courier"
  end -- if
  
  -- load fonts - mouseover window
  WindowCreate (infowin, 0, 0, 1, 1, 0, 0, 0)   -- make 1-pixel wide window
  
  -- install the fonts  (49 is modern / fixed pitch)
  WindowFont (infowin, font_id, font_name, font_size, false, false, false, false, 0, 49) 
  WindowFont (infowin, heading_font_id, font_name, font_size + 2, false, false, false, false, 0, 49)
  
    
  -- NOW DISPLAY A WINDOW
  
  -- what to say - one line per table entry, with imbedded colour codes
  
  info = { "@Ctesting 1 2 3", 
           "@GThis is a heading",
           "Line @Mwith @Bmultiple @Rcolours",
         }
  
  heading = "@MHello, @Yworld"
  left, top = 40, 50
  align_right = false
  align_bottom = false
  capitalize = true

  -- show it  
  mw.popup (infowin,           -- window name to use
            heading_font_id,   -- font to use for the heading
            font_id,           -- font to use for each line
            heading,           -- heading text
            info,              -- table of lines to show (with colour codes)
            left, top,         -- where to put it
            border_colour,     -- colour for round rectangle line
            background_colour, -- colour for background
            capitalize,        -- if true, force the first letter to upper case
            align_right,       -- if true, align right side on "Left" parameter
            align_bottom)      -- if true, align bottom side on "Top" parameter




--]]


module (..., package.seeall)

DEFAULT_COLOUR = "@w"
TRANSPARENCY_COLOUR = 0x080808
BORDER_WIDTH = 2

local BLACK = 1
local RED = 2
local GREEN = 3  
local YELLOW = 4 
local BLUE = 5 
local MAGENTA = 6 
local CYAN = 7 
local WHITE = 8

-- colour styles (eg. @r is normal red, @R is bold red)

-- @- is shown as ~
-- @@ is shown as @

-- This table uses the colours as defined in the MUSHclient ANSI tab, however the
-- defaults are shown on the right if you prefer to use those.

colour_conversion = {
   k = GetNormalColour (BLACK)   ,   -- 0x000000 
   r = GetNormalColour (RED)     ,   -- 0x000080 
   g = GetNormalColour (GREEN)   ,   -- 0x008000 
   y = GetNormalColour (YELLOW)  ,   -- 0x008080 
   b = GetNormalColour (BLUE)    ,   -- 0x800000 
   m = GetNormalColour (MAGENTA) ,   -- 0x800080 
   c = GetNormalColour (CYAN)    ,   -- 0x808000 
   w = GetNormalColour (WHITE)   ,   -- 0xC0C0C0 
   K = GetBoldColour   (BLACK)   ,   -- 0x808080 
   R = GetBoldColour   (RED)     ,   -- 0x0000FF 
   G = GetBoldColour   (GREEN)   ,   -- 0x00FF00 
   Y = GetBoldColour   (YELLOW)  ,   -- 0x00FFFF 
   B = GetBoldColour   (BLUE)    ,   -- 0xFF0000 
   M = GetBoldColour   (MAGENTA) ,   -- 0xFF00FF 
   C = GetBoldColour   (CYAN)    ,   -- 0xFFFF00 
   W = GetBoldColour   (WHITE)   ,   -- 0xFFFFFF 
   
   -- add custom colours here
   
   
  }  -- end conversion table

  
  
-- displays text with colour codes imbedded
-- 
-- win: window to use
-- font_id : font to use
-- Text : what to display
-- Left, Top, Right, Bottom : where to display it
-- Capitalize : if true, turn the first letter into upper-case

function colourtext (win, font_id, Text, Left, Top, Right, Bottom, Capitalize, utf8)

  if Text:match ("@") then
    local x = Left  -- current x position
    local need_caps = Capitalize
    
    Text = Text:gsub ("@%-", "~")    -- fix tildes
    Text = Text:gsub ("@@", "\0")  -- change @@ to 0x00
    
    -- make sure we start with @ or gsub doesn't work properly
    if Text:sub (1, 1) ~= "@" then
      Text = DEFAULT_COLOUR .. Text
    end -- if
    
    for colour, text in Text:gmatch ("@(%a)([^@]+)") do
      text = text:gsub ("%z", "@") -- put any @ characters back
      
      if need_caps then
        local count
        text, count = text:gsub ("%a", string.upper, 1)
        need_caps = count == 0 -- if not done, still need to capitalize yet
      end -- if
      
      if #text > 0 then
        x = x + WindowText (win, font_id, text, x, Top, Right, Bottom, 
                            colour_conversion [colour] or GetNormalColour (WHITE), utf8)
      end -- some text to display
          
    end -- for each colour run
    
    return x
  end -- if
  

  if Capitalize then
    Text = Text:gsub ("%a", string.upper, 1)
  end -- if leading caps wanted
  
  return WindowText (win, font_id, Text, Left, Top, Right, Bottom, 
                    colour_conversion [DEFAULT_COLOUR] or GetNormalColour (WHITE))

end -- colourtext

-- converts text with colour styles in it into style runs

function ColoursToStyles (Text)

 if Text:match ("@") then

    astyles = {}

    Text = Text:gsub ("@%-", "~")    -- fix tildes
    Text = Text:gsub ("@@", "\0")  -- change @@ to 0x00

    -- make sure we start with @ or gsub doesn't work properly
    if Text:sub (1, 1) ~= "@" then
       Text = DEFAULT_COLOUR .. Text
    end -- if

    for colour, text in Text:gmatch ("@(%a)([^@]+)") do

       text = text:gsub ("%z", "@") -- put any @ characters back

       if #text > 0 then
          table.insert (astyles, { text = text, 
                                   length = #text, 
                                   textcolour = colour_conversion [colour] or GetNormalColour (WHITE),
                                   backcolour = GetNormalColour (BLACK) })
       end -- if some text
    end -- for each colour run.

    return astyles

 end -- if any colour codes at all

 -- No colour codes, create a single style.
 return { { text = Text, 
            length = #Text, 
            textcolour = GetNormalColour (WHITE),
            backcolour = GetNormalColour (BLACK) } }

end  -- function ColoursToStyles

-- take a string, and remove colour codes from it (eg. "@Ghello" becomes "hello"
function strip_colours (s)
  s = s:gsub ("@%-", "~")    -- fix tildes
  s = s:gsub ("@@", "\0")  -- change @@ to 0x00
  s = s:gsub ("@%a([^@]*)", "%1")
  return (s:gsub ("%z", "@")) -- put @ back
end -- strip_colours


function popup (win,    -- window name to use
                heading_font_id,   -- font to use for the heading
                font_id,           -- font to use for each line
                heading,           -- heading text
                info,              -- table of lines to show (with colour codes)
                Left, Top,         -- where to put it
                border_colour,     -- colour for round rectangle line
                background_colour, -- colour for background
                capitalize,        -- if true, force the first letter to be upper case
                align_right,       -- if true, align right side on "Left" parameter
                align_bottom)      -- if true, align bottom side on "Top" parameter

--[[

  A prerequisite is to create the window first and load the two fonts into it.
  This only needs to be done once, and as it may take a bit of code to select a font that actually
  exists, I prefer to not do it here.
  
  Example code:

  infowin = GetPluginID () .. ":info"
  font_id = "popup_font"  
  heading_font_id = "popup_heading_font"  

  font_size = 8

  -- use 8 pt Dina or Lucida Sans, or 10 pt Courier
  local fonts = utils.getfontfamilies ()

  if fonts.Dina then
    font_name = "Dina"    
  elseif fonts ["Lucida Sans Unicode"] then
    font_name = "Lucida Sans Unicode"
  else
    font_size = 10
    font_name = "Courier"
  end -- if

  
  -- load fonts - mouseover window
  WindowCreate (infowin, 0, 0, 1, 1, 0, 0, 0)   -- make 1-pixel wide window
  
  -- install the fonts  (49 is modern / fixed pitch)
  WindowFont (infowin, font_id, font_name, font_size, false, false, false, false, 0, 49) 
  WindowFont (infowin, heading_font_id, font_name, font_size + 2, false, false, false, false, 0, 49)
  

--]]

                
  assert (WindowInfo (win, 1), "Window " .. win .. " must already exist")
  assert (WindowFontInfo (win, heading_font_id, 1), "No font " .. heading_font_id .. " in " .. win)
  assert (WindowFontInfo (win, font_id, 1), "No font " .. font_id .. " in " .. win)
  
  local font_height = WindowFontInfo (win, font_id, 1)
  local font_leading = WindowFontInfo (win, font_id, 4) + WindowFontInfo (win, font_id, 5)
  local heading_font_height = WindowFontInfo (win, heading_font_id, 1)
  
  -- find text width - minus colour codes  
  
  local infowidth = 0 
  local infoheight = 0 
  
  -- calculate heading width and height
  if heading and #heading > 0 then
    infowidth  = WindowTextWidth (win, heading_font_id, strip_colours (heading))
    infoheight = heading_font_height
  end -- have a heading

  -- calculate remaining width and height
  for _, v in ipairs (info) do
    infowidth  = math.max (infowidth, WindowTextWidth (win, font_id, strip_colours (v)))
    infoheight = infoheight + font_height
  end -- for

  infowidth = infowidth + (2 * BORDER_WIDTH) +    -- leave room for border
                WindowFontInfo (win, font_id, 6)  -- one character width extra
                
  infoheight = infoheight + (2 * BORDER_WIDTH) +  -- leave room for border
                font_leading +                    -- plus leading below bottom line, 
                10                                -- and 5 pixels top and bottom
  
  if align_right then
    Left = Left - infowidth 
  end -- if align_right
  
  if align_bottom then
    Top = Top - infoheight 
  end -- if align_bottom
  
  WindowCreate (win, 
                Left, Top,    -- where
                infowidth,    -- width  (gap of 5 pixels per side)
                infoheight,   -- height
                4,            -- position mode: can't be 0 to 3
                2 + 4,        -- absolute location + transparent
                TRANSPARENCY_COLOUR)   -- background (transparent) colour
  
  WindowCircleOp (win, 3, -- round rectangle
        BORDER_WIDTH, BORDER_WIDTH, -BORDER_WIDTH, -BORDER_WIDTH,  -- border inset
        border_colour, 0, BORDER_WIDTH,  -- solid line
        background_colour, 0, -- solid
        5, 5)  -- diameter of ellipse

  local x = BORDER_WIDTH + WindowFontInfo (win, font_id, 6) / 2    -- start 1/2 character in 
  local y = BORDER_WIDTH + 5          -- skip border, and leave 5 pixel gap
         
  -- heading if wanted
  if heading and #heading > 0 then
    colourtext (win, heading_font_id, heading, x, y, 0, 0, capitalize)
    y = y + heading_font_height
  end -- have a heading
     
  -- show each line
  for _, v in ipairs (info) do
    colourtext (win, font_id, v, x, y, 0, 0, capitalize)
    y = y + font_height
  end -- for
  
  -- display popup window
  WindowShow (win, true)

end -- popup

