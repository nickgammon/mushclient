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
   mw.tooltip           - make a tooltip window

  EXAMPLE OF MAKING A POPUP WINDOW:

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

  -- load fonts
  WindowCreate (infowin, 0, 0, 0, 0, 0, 0, 0)   -- make initial window

  -- install the fonts
  WindowFont (infowin, font_id, font_name, font_size, false, false, false, false,
              miniwin.font_charset_ansi,
              miniwin.font_family_modern + miniwin.font_pitch_fixed)
  WindowFont (infowin, heading_font_id, font_name, font_size + 2, false, false, false, false,
              miniwin.font_charset_ansi,
              miniwin.font_family_modern + miniwin.font_pitch_fixed)

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


  EXAMPLE OF MAKING A TOOLTIP WINDOW:

  -- SET UP FOR TOOLTIP WINDOWS - define colours, add fonts, make window id
  -- (DO THIS ONCE ONLY, eg. in OnPluginInstall)

  -- Example setup code:
  require "mw"
  -- get ready for tooltips
  win_tooltip       = GetPluginID () .. "_tooltip"
  font_tooltip      = "tf"
  bold_font_tooltip = "tfb"
  -- make the window
  WindowCreate (win_tooltip, 0, 0, 0, 0, 0, 0, 0)
  -- load some fonts into it
  WindowFont (win_tooltip, font_tooltip,      "Tahoma", 8)
  WindowFont (win_tooltip, bold_font_tooltip, "Tahoma", 8, true)

  -- NOW DISPLAY A tooltip (Put bold lines inside asterisks)

  mw.tooltip (win_tooltip,     -- window name to use
              font_tooltip,      -- font to use for each line
              bold_font_tooltip, -- bold font
              "*Info*\nHello, world!\nHave fun.",        -- tooltip text
              45, 75,      -- where to put it (x, y)
              0,  -- colour for text   (black)
              0,  -- colour for border (black)
              ColourNameToRGB ("#FFFFE1")) -- colour for background

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
                miniwin.pos_top_left,  -- position mode: can't be 0 to 3
                miniwin.create_absolute_location + miniwin.create_transparent,
                TRANSPARENCY_COLOUR)   -- background (transparent) colour

  WindowCircleOp (win, miniwin.circle_round_rectangle,
        BORDER_WIDTH, BORDER_WIDTH, -BORDER_WIDTH, -BORDER_WIDTH,  -- border inset
        border_colour, miniwin.pen_solid, BORDER_WIDTH,  -- line
        background_colour, miniwin.brush_solid,          -- fill
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

-- --------------------------------------------------------------
-- Displays a tooltip (small box with "arrow" pointing to something of interest)
-- Bold lines are surrounded by asterisks (eg. *Info*)
-- --------------------------------------------------------------
function tooltip (win,    -- window name to use
                  font_id,        -- font to use for each line
                  bold_font_id,   -- font to use for bold lines
                  text,           -- tooltip text
                  Left, Top,      -- where to put it (x, y)
                  text_colour,    -- colour for text
                  border_colour,  -- colour for border
                  background_colour) -- colour for background

  assert (WindowInfo (win, 1), "Window " .. win .. " must already exist")
  assert (WindowFontInfo (win, font_id, 1), "No font " .. font_id .. " in " .. win)
  assert (WindowFontInfo (win, bold_font_id, 1), "No font " .. bold_font_id .. " in " .. win)
  local font_height      = WindowFontInfo (win, font_id, 1)
  local bold_font_height = WindowFontInfo (win, bold_font_id, 1)
  local MARGIN = 8
  local TIPSIZE = 12

  -- break text into lines
  local t = utils.split (text, "\n")

  -- tooltip height
  local height = MARGIN * 2  -- margin at top and bottom
  -- tooltip width
  local width = TIPSIZE * 2  -- must be at least large enough for the tip part
  for k, v in ipairs (t) do
    -- bold lines start and end with an asterisk
    local boldText = string.match (v, "^%*(.*)%*$")
    if boldText then
      width = math.max (width, WindowTextWidth (win, bold_font_id, boldText, true))
      height = height + bold_font_height
    else
      width = math.max (width, WindowTextWidth (win, font_id, v, true))
      height = height + font_height
    end -- if
  end -- for
  width = width + (MARGIN * 2)  -- margin per side

  -- the tooltip pointer starts TIPSIZE pixels to the right and descends TIPSIZE pixels
  WindowCreate (win,
                Left - TIPSIZE, Top - height - TIPSIZE,
                width + 2, height + TIPSIZE + 2,  -- 2 pixels margin to allow for border + TIPSIZE downwards for the tip
                miniwin.pos_top_left,    -- position
                miniwin.create_absolute_location + miniwin.create_transparent + miniwin.create_ignore_mouse,
                TRANSPARENCY_COLOUR)

  -- mucking around here to get rounded rectangle
  local points = {
    -- top LH corner
    1, 6,
    2, 6,
    2, 4,
    3, 4,
    3, 3,
    4, 3,
    4, 2,
    6, 2,
    6, 1,

    -- top RH corner
    width - 5, 1,
    width - 5, 2,
    width - 3, 2,
    width - 3, 3,
    width - 2, 3,
    width - 2, 4,
    width - 1, 4,
    width - 1, 6,
    width,     6,

    -- bottom RH corner
    width,     height - 5,
    width - 1, height - 5,
    width - 1, height - 3,
    width - 2, height - 3,
    width - 2, height - 2,
    width - 3, height - 2,
    width - 3, height - 1,
    width - 5, height - 1,
    width - 5, height,

    (TIPSIZE * 2) + 1, height,     -- RH side of tip
    TIPSIZE + 1, height + TIPSIZE, -- bottom of  tip
    TIPSIZE + 1, height,           -- LH side of tip

    -- bottom LH corner
    6, height,
    6, height - 1,
    4, height - 1,
    4, height - 2,
    3, height - 2,
    3, height - 3,
    2, height - 3,
    2, height - 5,
    1, height - 5,
    }

  -- make the tooltip polygon
  WindowPolygon(win, table.concat (points, ","),
                border_colour,          -- pen colour
                miniwin.pen_solid, 1,   -- pen (1 pixel wide)
                background_colour,      -- brush colour
                miniwin.brush_solid,    -- brush
                true)  -- close it

  -- put the text into it
  local top = MARGIN + 1
  for _, v in ipairs (t) do
    -- bold lines start and end with an asterisk
    local boldText = string.match (v, "^%*(.*)%*$")
    if boldText then
      WindowText (win, bold_font_id, boldText, MARGIN + 1, top, 0, 0, text_colour, true)
      top = top + bold_font_height
    else
      WindowText (win, font_id, v, MARGIN + 1, top, 0, 0, text_colour, true)
      top = top + font_height
    end -- if
  end -- for

  WindowShow (win, true)

end -- tooltip