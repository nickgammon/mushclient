-- mapper.lua

--[[

Author: Nick Gammon
Date:   11th March 2010
Amended: 15th August 2010
Amended: 2nd October 2010
Amended: 18th October 2010 to added find callback
Amended: 16th November 2010 to add symbolic constants (miniwin.xxxx)
Amended: 18th November 2010 to add more timing and count of times called
         Also added zooming with the mouse wheel.
Amended: 26th November 2010 to check timers are enabled when speedwalking.
Amended: 11th November 2014 to allow for detecting mouse-overs of rooms

Generic MUD mapper.

Exposed functions:

init (t)            -- call once, supply:
                          t.config      -- ie. colours, sizes
                          t.get_room    -- info about room (uid)
                          t.show_help   -- function that displays some help
                          t.room_click  -- function that handles RH click on room (uid, flags)
                          t.room_mouseover -- function that handles mouse-over a room (uid, flags)
                          t.room_cancelmouseover -- function that handles cancelled mouse-over of a room (uid, flags)
                          t.timing      -- true to show timing
                          t.show_completed  -- true to show "Speedwalk completed."
                          t.show_other_areas -- true to show non-current areas
                          t.show_up_down    -- follow up/down exits
                          t.show_area_exits  -- true to draw a circle around rooms leading to other areas
                          t.speedwalk_prefix   -- if not nil, speedwalk by prefixing with this

zoom_in ()          -- zoom in map view
zoom_out ()         -- zoom out map view
mapprint (message)  -- like print, but uses mapper colour
maperror (message)  -- like print, but prints in red
hide ()             -- hides map window (eg. if plugin disabled)
show ()             -- show map window  (eg. if plugin enabled)
save_state ()       -- call to save plugin state (ie. in OnPluginSaveState)
draw (uid)          -- draw map - starting at room 'uid'
start_speedwalk (path)  -- starts speedwalking. path is a table of directions/uids
build_speedwalk (path)  -- builds a client speedwalk string from path
cancel_speedwalk ()     -- cancel current speedwalk, if any
check_we_can_find ()    -- returns true if doing a find is OK right now
find (f, show_uid, count, walk)      -- generic room finder
find_paths (uid, f)     -- lower-level room finder (for getting back a path)

Exposed variables:

win                 -- the window (in case you want to put up menus)
VERSION             -- mapper version
last_hyperlink_uid  -- room uid of last hyperlink click (destination)
last_speedwalk_uid  -- room uid of last speedwalk attempted (destination)
<various functions> -- functions required to be global by the client (eg. for mouseup)

Room info should include:

  name          (what to show as room name)
  exits         (table keyed by direction, value is exit uid)
  area          (area name)
  hovermessage  (what to show when you mouse-over the room)
  bordercolour  (colour of room border)     - RGB colour
  borderpen     (pen style of room border)  - see WindowCircleOp (values 0 to 6)
  borderpenwidth(pen width of room border)  - eg. 1 for normal, 2 for current room
  fillcolour    (colour to fill room)       - RGB colour, nil for default
  fillbrush     (brush to fill room)        - see WindowCircleOp (values 0 to 12)

--]]

module (..., package.seeall)

VERSION = 2.6   -- for querying by plugins

require "movewindow"
require "copytable"
require "gauge"
require "pairsbykeys"
require "mw"

local FONT_ID     = "fn"  -- internal font identifier
local FONT_ID_UL  = "fnu" -- internal font identifier - underlined

-- size of room box
local ROOM_SIZE = 10

-- how far away to draw rooms from each other
local DISTANCE_TO_NEXT_ROOM = 15

-- supplied in init
local config  -- configuration table
local supplied_get_room
local room_click
local room_mouseover
local room_cancelmouseover
local timing            -- true to show timing and other info
local show_completed    -- true to show "Speedwalk completed."
local show_other_areas  -- true to draw other areas
local show_area_exits   -- true to show area exits
local show_up_down      -- true to show up/down exits

-- current room number
local current_room

-- our copy of rooms info
local rooms = {}
local last_visited = {}

-- other locals
local HALF_ROOM, connectors, half_connectors, arrows
local plan_to_draw, speedwalks, drawn, drawn_coords
local last_drawn, depth, windowinfo, font_height
local walk_to_room_name
local total_times_drawn = 0
local total_time_taken = 0

local function build_room_info ()

  HALF_ROOM   = ROOM_SIZE / 2
  local THIRD_WAY   = DISTANCE_TO_NEXT_ROOM / 3
  local DISTANCE_LESS1 = DISTANCE_TO_NEXT_ROOM - 1

  -- how to draw a line from this room to the next one (relative to the center of the room)
  connectors = {
    n =  { x1 = 0,            y1 = - HALF_ROOM, x2 = 0,                             y2 = - HALF_ROOM - DISTANCE_LESS1, at = { 0, -1 } },
    s =  { x1 = 0,            y1 =   HALF_ROOM, x2 = 0,                             y2 =   HALF_ROOM + DISTANCE_LESS1, at = { 0,  1 } },
    e =  { x1 =   HALF_ROOM,  y1 = 0,           x2 =   HALF_ROOM + DISTANCE_LESS1,  y2 = 0,                            at = {  1,  0 }},
    w =  { x1 = - HALF_ROOM,  y1 = 0,           x2 = - HALF_ROOM - DISTANCE_LESS1,  y2 = 0,                            at = { -1,  0 }},

    ne = { x1 =   HALF_ROOM,  y1 = - HALF_ROOM, x2 =   HALF_ROOM + DISTANCE_LESS1 , y2 = - HALF_ROOM - DISTANCE_LESS1, at = { 1, -1 } },
    se = { x1 =   HALF_ROOM,  y1 =   HALF_ROOM, x2 =   HALF_ROOM + DISTANCE_LESS1 , y2 =   HALF_ROOM + DISTANCE_LESS1, at = { 1,  1 } },
    nw = { x1 = - HALF_ROOM,  y1 = - HALF_ROOM, x2 = - HALF_ROOM - DISTANCE_LESS1 , y2 = - HALF_ROOM - DISTANCE_LESS1, at = {-1, -1 } },
    sw = { x1 = - HALF_ROOM,  y1 =   HALF_ROOM, x2 = - HALF_ROOM - DISTANCE_LESS1 , y2 =   HALF_ROOM + DISTANCE_LESS1, at = {-1,  1 } },

    } -- end connectors

  -- how to draw a stub line
  half_connectors = {
    n =  { x1 = 0,            y1 = - HALF_ROOM, x2 = 0,                        y2 = - HALF_ROOM - THIRD_WAY, at = { 0, -1 } },
    s =  { x1 = 0,            y1 =   HALF_ROOM, x2 = 0,                        y2 =   HALF_ROOM + THIRD_WAY, at = { 0,  1 } },
    e =  { x1 =   HALF_ROOM,  y1 = 0,           x2 =   HALF_ROOM + THIRD_WAY,  y2 = 0,                       at = {  1,  0 }},
    w =  { x1 = - HALF_ROOM,  y1 = 0,           x2 = - HALF_ROOM - THIRD_WAY,  y2 = 0,                       at = { -1,  0 }},

    ne = { x1 =   HALF_ROOM,  y1 = - HALF_ROOM, x2 =   HALF_ROOM + THIRD_WAY , y2 = - HALF_ROOM - THIRD_WAY, at = { 1, -1 } },
    se = { x1 =   HALF_ROOM,  y1 =   HALF_ROOM, x2 =   HALF_ROOM + THIRD_WAY , y2 =   HALF_ROOM + THIRD_WAY, at = { 1,  1 } },
    nw = { x1 = - HALF_ROOM,  y1 = - HALF_ROOM, x2 = - HALF_ROOM - THIRD_WAY , y2 = - HALF_ROOM - THIRD_WAY, at = {-1, -1 } },
    sw = { x1 = - HALF_ROOM,  y1 =   HALF_ROOM, x2 = - HALF_ROOM - THIRD_WAY , y2 =   HALF_ROOM + THIRD_WAY, at = {-1,  1 } },

    } -- end half_connectors

  -- how to draw one-way arrows (relative to the center of the room)
  arrows = {
     n =  { - 2, - HALF_ROOM - 2,  2, - HALF_ROOM - 2,  0, - HALF_ROOM - 6 },
     s =  { - 2,   HALF_ROOM + 2,  2,   HALF_ROOM + 2,  0,   HALF_ROOM + 6  },
     e =  {   HALF_ROOM + 2, -2,   HALF_ROOM + 2, 2,   HALF_ROOM + 6, 0 },
     w =  { - HALF_ROOM - 2, -2, - HALF_ROOM - 2, 2, - HALF_ROOM - 6, 0 },

     ne = {   HALF_ROOM + 3,  - HALF_ROOM,  HALF_ROOM + 3, - HALF_ROOM - 3,  HALF_ROOM, - HALF_ROOM - 3 },
     se = {   HALF_ROOM + 3,    HALF_ROOM,  HALF_ROOM + 3,   HALF_ROOM + 3,  HALF_ROOM,   HALF_ROOM + 3 },
     nw = { - HALF_ROOM - 3,  - HALF_ROOM,  - HALF_ROOM - 3, - HALF_ROOM - 3,  - HALF_ROOM, - HALF_ROOM - 3 },
     sw = { - HALF_ROOM - 3,    HALF_ROOM,  - HALF_ROOM - 3,   HALF_ROOM + 3,  - HALF_ROOM,   HALF_ROOM + 3},

    } -- end of arrows

end -- build_room_info

local default_config = {
  -- assorted colours
  BACKGROUND_COLOUR       = { name = "Background",        colour =  ColourNameToRGB "lightseagreen", },
  ROOM_COLOUR             = { name = "Room",              colour =  ColourNameToRGB "cyan", },
  EXIT_COLOUR             = { name = "Exit",              colour =  ColourNameToRGB "darkgreen", },
  EXIT_COLOUR_UP_DOWN     = { name = "Exit up/down",      colour =  ColourNameToRGB "darkmagenta", },
  EXIT_COLOUR_IN_OUT      = { name = "Exit in/out",       colour =  ColourNameToRGB "#3775E8", },
  UNKNOWN_ROOM_COLOUR     = { name = "Unknown room",      colour =  ColourNameToRGB "#00CACA", },
  MAPPER_NOTE_COLOUR      = { name = "Messages",          colour =  ColourNameToRGB "lightgreen" },

  ROOM_NAME_TEXT          = { name = "Room name text",    colour = ColourNameToRGB "#BEF3F1", },
  ROOM_NAME_FILL          = { name = "Room name fill",    colour = ColourNameToRGB "#105653", },
  ROOM_NAME_BORDER        = { name = "Room name box",     colour = ColourNameToRGB "black", },

  AREA_NAME_TEXT          = { name = "Area name text",    colour = ColourNameToRGB "#BEF3F1",},
  AREA_NAME_FILL          = { name = "Area name fill",    colour = ColourNameToRGB "#105653", },
  AREA_NAME_BORDER        = { name = "Area name box",     colour = ColourNameToRGB "black", },

  FONT = { name =  get_preferred_font {"Dina",  "Lucida Console",  "Fixedsys", "Courier", "Sylfaen",} ,
           size = 8
         } ,

  -- size of map window
  WINDOW = { width = 400, height = 400 },

  -- how far from where we are standing to draw (rooms)
  SCAN = { depth = 30 },

  -- speedwalk delay
  DELAY = { time = 0.3 },

  -- how many seconds to show "recent visit" lines (default 3 minutes)
  LAST_VISIT_TIME = { time = 60 * 3 },

  }

local expand_direction = {
  n = "north",
  s = "south",
  e = "east",
  w = "west",
  u = "up",
  d = "down",
  ne = "northeast",
  sw = "southwest",
  nw = "northwest",
  se = "southeast",
  ['in'] = "in",
  out = "out",
  }  -- end of expand_direction

local function get_room (uid)
  local room = supplied_get_room (uid)
  room = room or { unknown = true }

  -- defaults in case they didn't supply them ...
  room.name = room.name or string.format ("Room %s", uid or "<none>")
  room.name = mw.strip_colours (room.name)  -- no colour codes for now
  room.exits = room.exits or {}
  room.area = room.area or "<No area>"
  room.hovermessage = room.hovermessage or "<Unexplored room>"
  room.bordercolour = room.bordercolour or config.ROOM_COLOUR.colour
  room.borderpen = room.borderpen or 0 -- solid
  room.borderpenwidth = room.borderpenwidth or 1
  room.fillcolour = room.fillcolour or 0x000000
  room.fillbrush = room.fillbrush or 1 -- no fill

  return room
end -- get_room

local function check_connected ()
  if not IsConnected() then
    mapprint ("You are not connected to", WorldName())
    return false
  end -- if not connected
  return true
end -- check_connected

local function make_number_checker (title, min, max, decimals)
  return function (s)
    local n = tonumber (s)
    if not n then
      utils.msgbox (title .. " must be a number", "Incorrect input", "ok", "!", 1)
      return false  -- bad input
    end -- if

    if n < min or n > max then
      utils.msgbox (title .. " must be in range " .. min .. " to " .. max, "Incorrect input", "ok", "!", 1)
      return false  -- bad input
    end -- if

    if not decimals then
      if string.match (s, "%.") then
        utils.msgbox (title .. " cannot have decimal places", "Incorrect input", "ok", "!", 1)
        return false  -- bad input
      end -- if
    end -- no decimals

    return true  -- good input
  end -- generated function

end -- make_number_checker


local function get_number_from_user (msg, title, current, min, max, decimals)
  local max_length = math.ceil (math.log10 (max) + 1)

  -- if decimals allowed, allow room for them
  if decimals then
    max_length = max_length + 2  -- allow for 0.x
  end -- if

  -- if can be negative, allow for minus sign
  if min < 0 then
    max_length = max_length + 1
  end -- if can be negative

  return tonumber (utils.inputbox (msg, title, current, nil, nil,
                      { validate = make_number_checker (title, min, max, decimals),
                        prompt_height = 14,
                        box_height = 130,
                        box_width = 300,
                        reply_width = 150,
                        max_length = max_length,
                      }  -- end extra stuff
                   ))
end -- get_number_from_user

local function draw_configuration ()
  local width =  max_text_width (win, FONT_ID, {"Configuration", "Font", "Width", "Height", "Depth"}, true)
  local lines = 6  -- "Configuration", font, width, height, depth, delay
  local GAP = 5
  local suppress_colours = false

  for k, v in pairs (config) do
    if v.colour then
      width = math.max (width, WindowTextWidth (win, FONT_ID, v.name, true))
      lines = lines + 1
    end -- a colour item
  end -- for each config item

  if (config.WINDOW.height - 13 - font_height * lines) < 10 then
    suppress_colours = true
    lines = 6  -- forget all the colours
  end -- if

  local x = 3
  local y = config.WINDOW.height - 13 - font_height * lines
  local box_size = font_height - 2
  local rh_size = math.max (box_size, max_text_width (win, FONT_ID,
    {config.FONT.name .. " " .. config.FONT.size,
     tostring (config.WINDOW.width),
     tostring (config.WINDOW.height),
     tostring (config.SCAN.depth)},
    true))
  local frame_width = GAP + width + GAP + rh_size + GAP  -- gap / text / gap / box / gap

  -- fill entire box with grey
  WindowRectOp (win, miniwin.rect_fill, x, y, x + frame_width, y + font_height * lines + 10, 0xDCDCDC)
  -- frame it
  draw_3d_box (win, x, y, frame_width, font_height * lines + 10)

  y = y + GAP
  x = x + GAP

  -- title
  WindowText   (win, FONT_ID, "Configuration", x, y, 0, 0, 0x808080, true)

  -- close box
  WindowRectOp (win,
                miniwin.rect_frame,
                x + frame_width - box_size - GAP * 2,
                y + 1,
                x + frame_width - GAP * 2,
                y + 1 + box_size,
                0x808080)
  WindowLine (win,
              x + frame_width - box_size - GAP * 2 + 3,
              y + 4,
              x + frame_width - GAP * 2 - 3,
              y - 2 + box_size,
              0x808080,
              miniwin.pen_solid, 1)
  WindowLine (win,
              x - 4 + frame_width - GAP * 2,
              y + 4,
              x - 1 + frame_width - box_size - GAP * 2 + 3,
              y - 2 + box_size,
              0x808080,
              miniwin.pen_solid, 1)

  -- close configuration hotspot
  WindowAddHotspot(win, "$<close_configure>",
                   x + frame_width - box_size - GAP * 2,
                   y + 1,
                   x + frame_width - GAP * 2,
                   y + 1 + box_size,   -- rectangle
                   "", "", "", "", "mapper.mouseup_close_configure",  -- mouseup
                   "Click to close",
                   miniwin.cursor_hand, 0)  -- hand cursor

  y = y + font_height

  if not suppress_colours then

    for k, v in pairsByKeys (config) do
      if v.colour then
        WindowText   (win, FONT_ID, v.name, x, y, 0, 0, 0x000000, true)
        WindowRectOp (win,
                      miniwin.rect_fill,
                      x + width + rh_size / 2,
                      y + 1,
                      x + width + rh_size / 2 + box_size,
                      y + 1 + box_size,
                      v.colour)
        WindowRectOp (win,
                      miniwin.rect_frame,
                      x + width + rh_size / 2,
                      y + 1,
                      x + width + rh_size / 2 + box_size,
                      y + 1 + box_size,
                      0x000000)

        -- colour change hotspot
        WindowAddHotspot(win,
                         "$colour:" .. k,
                         x + GAP,
                         y + 1,
                         x + width + rh_size / 2 + box_size,
                         y + 1 + box_size,   -- rectangle
                         "", "", "", "", "mapper.mouseup_change_colour",  -- mouseup
                         "Click to change colour",
                         miniwin.cursor_hand, 0)  -- hand cursor

        y = y + font_height
      end -- a colour item
    end -- for each config item
  end -- if

  -- depth
  WindowText   (win, FONT_ID, "Depth", x, y, 0, 0, 0x000000, true)
  WindowText   (win, FONT_ID_UL, tostring (config.SCAN.depth), x + width + GAP, y, 0, 0, 0x808080, true)

  -- depth hotspot
  WindowAddHotspot(win,
                   "$<depth>",
                   x + GAP,
                   y,
                   x + frame_width,
                   y + font_height,   -- rectangle
                   "", "", "", "", "mapper.mouseup_change_depth",  -- mouseup
                   "Click to change scan depth",
                   miniwin.cursor_hand, 0)  -- hand cursor
  y = y + font_height

  -- font
  WindowText   (win, FONT_ID, "Font", x, y, 0, 0, 0x000000, true)
  WindowText   (win, FONT_ID_UL, config.FONT.name .. " " .. config.FONT.size, x + width + GAP, y, 0, 0, 0x808080, true)

  -- colour font hotspot
  WindowAddHotspot(win,
                   "$<font>",
                   x + GAP,
                   y,
                   x + frame_width,
                   y + font_height,   -- rectangle
                   "", "", "", "", "mapper.mouseup_change_font",  -- mouseup
                   "Click to change font",
                   miniwin.cursor_hand, 0)  -- hand cursor
  y = y + font_height


  -- width
  WindowText   (win, FONT_ID, "Width", x, y, 0, 0, 0x000000, true)
  WindowText   (win, FONT_ID_UL, tostring (config.WINDOW.width), x + width + GAP, y, 0, 0, 0x808080, true)

  -- width hotspot
  WindowAddHotspot(win,
                   "$<width>",
                   x + GAP,
                   y,
                   x + frame_width,
                   y + font_height,   -- rectangle
                   "", "", "", "", "mapper.mouseup_change_width",  -- mouseup
                   "Click to change window width",
                   miniwin.cursor_hand, 0)  -- hand cursor
  y = y + font_height

  -- height
  WindowText   (win, FONT_ID, "Height", x, y, 0, 0, 0x000000, true)
  WindowText   (win, FONT_ID_UL, tostring (config.WINDOW.height), x + width + GAP, y, 0, 0, 0x808080, true)

  -- height hotspot
  WindowAddHotspot(win,
                   "$<height>",
                   x + GAP,
                   y,
                   x + frame_width,
                   y + font_height,   -- rectangle
                   "", "", "", "", "mapper.mouseup_change_height",  -- mouseup
                   "Click to change window height",
                   miniwin.cursor_hand, 0)  -- hand cursor
  y = y + font_height

  -- delay
  WindowText   (win, FONT_ID, "Walk delay", x, y, 0, 0, 0x000000, true)
  WindowText   (win, FONT_ID_UL, tostring (config.DELAY.time), x + width + GAP, y, 0, 0, 0x808080, true)

  -- height hotspot
  WindowAddHotspot(win,
                   "$<delay>",
                   x + GAP,
                   y,
                   x + frame_width,
                   y + font_height,   -- rectangle
                   "", "", "", "", "mapper.mouseup_change_delay",  -- mouseup
                   "Click to change speedwalk delay",
                   miniwin.cursor_hand, 0)  -- hand cursor
  y = y + font_height

end -- draw_configuration

-- for calculating one-way paths
local inverse_direction = {
  n = "s",
  s = "n",
  e = "w",
  w = "e",
  u = "d",
  d = "u",
  ne = "sw",
  sw = "ne",
  nw = "se",
  se = "nw",
  ['in'] = "out",
  out = "in",
  }  -- end of inverse_direction

local function add_another_room (uid, path, x, y)
  local path = path or {}
  return {uid=uid, path=path, x = x, y = y}
end  -- add_another_room

local function draw_room (uid, path, x, y)

  local coords = string.format ("%i,%i", math.floor (x), math.floor (y))

  -- need this for the *current* room !!!
  drawn_coords [coords] = uid

  -- print ("drawing", uid, "at", coords)

  if drawn [uid] then
    return
  end -- done this one

  -- don't draw the same room more than once
  drawn [uid] = { coords = coords, path = path }

  local room = rooms [uid]

  -- not cached - get from caller
  if not room then
    room = get_room (uid)
    rooms [uid] = room
  end -- not in cache


  local left, top, right, bottom = x - HALF_ROOM, y - HALF_ROOM, x + HALF_ROOM, y + HALF_ROOM

  -- forget it if off screen
  if x < HALF_ROOM or y < HALF_ROOM or
     x > config.WINDOW.width - HALF_ROOM or y > config.WINDOW.height - HALF_ROOM then
     return
  end -- if

  -- exits

  local texits = {}

  for dir, exit_uid in pairs (room.exits) do
    table.insert (texits, dir)
    local exit_info = connectors [dir]
    local stub_exit_info = half_connectors [dir]
    local exit_line_colour = config.EXIT_COLOUR.colour
    local arrow = arrows [dir]

    -- draw up in the ne/nw position if not already an exit there at this level
    if dir == "u" then
      if not room.exits.nw then
        exit_info = connectors.nw
        stub_exit_info = half_connectors.nw
        arrow = arrows.nw
        exit_line_colour = config.EXIT_COLOUR_UP_DOWN.colour
      end -- if available
    elseif dir == "in" then
      if not room.exits.ne then
        exit_info = connectors.ne
        stub_exit_info = half_connectors.ne
        arrow = arrows.ne
        exit_line_colour = config.EXIT_COLOUR_IN_OUT.colour
      end -- if
    elseif dir == "d" then
      if not room.exits.se then
        exit_info = connectors.se
        stub_exit_info = half_connectors.se
        arrow = arrows.se
        exit_line_colour = config.EXIT_COLOUR_UP_DOWN.colour
      end -- if available
    elseif dir == "out" then
      if not room.exits.sw then
        exit_info = connectors.sw
        stub_exit_info = half_connectors.sw
        arrow = arrows.sw
        exit_line_colour = config.EXIT_COLOUR_IN_OUT.colour
      end -- if
    end -- if down

    if exit_info then
      local linetype = miniwin.pen_solid -- unbroken
      local linewidth = 1 -- not recent

      -- try to cache room
      if not rooms [exit_uid] then
        rooms [exit_uid] = get_room (exit_uid)
      end -- if

      if rooms [exit_uid].unknown then
        linetype = miniwin.pen_dot -- dots
      end -- if

      local next_x = x + exit_info.at [1] * (ROOM_SIZE + DISTANCE_TO_NEXT_ROOM)
      local next_y = y + exit_info.at [2] * (ROOM_SIZE + DISTANCE_TO_NEXT_ROOM)

      local next_coords = string.format ("%i,%i", math.floor (next_x), math.floor (next_y))

      -- remember if a zone exit (first one only)
      if show_area_exits and room.area ~= rooms [exit_uid].area then
        area_exits [ rooms [exit_uid].area ] = area_exits [ rooms [exit_uid].area ] or {x = x, y = y}
      end -- if

      -- if another room (not where this one leads to) is already there, only draw "stub" lines
      if drawn_coords [next_coords] and drawn_coords [next_coords] ~= exit_uid then
        exit_info = stub_exit_info
      elseif exit_uid == uid then

        -- here if room leads back to itself
        exit_info = stub_exit_info
        linetype = miniwin.pen_dash -- dash

      else
        if (not show_other_areas and rooms [exit_uid].area ~= current_area) or
           (not show_up_down and (dir == "u" or dir == "d")) then
            exit_info = stub_exit_info    -- don't show other areas
        else
          -- if we are scheduled to draw the room already, only draw a stub this time
          if plan_to_draw [exit_uid] and plan_to_draw [exit_uid] ~= next_coords then
            -- here if room already going to be drawn
            exit_info = stub_exit_info
            linetype = miniwin.pen_dash -- dash
          else
            -- remember to draw room next iteration
            local new_path = copytable.deep (path)
            table.insert (new_path, { dir = dir, uid = exit_uid })
            table.insert (rooms_to_be_drawn, add_another_room (exit_uid, new_path, next_x, next_y))
            drawn_coords [next_coords] = exit_uid
            plan_to_draw [exit_uid] = next_coords

            -- if exit room known
            if not rooms [exit_uid].unknown then
              local exit_time = last_visited [exit_uid] or 0
              local this_time = last_visited [uid] or 0
              local now = os.time ()
              if exit_time > (now - config.LAST_VISIT_TIME.time) and
                 this_time > (now - config.LAST_VISIT_TIME.time) then
                 linewidth = 2
              end -- if
            end -- if
          end -- if
        end -- if
      end -- if drawn on this spot

      WindowLine (win, x + exit_info.x1, y + exit_info.y1, x + exit_info.x2, y + exit_info.y2, exit_line_colour, linetype, linewidth)

      -- one-way exit?

      if not rooms [exit_uid].unknown then
        local dest = rooms [exit_uid]
        -- if inverse direction doesn't point back to us, this is one-way
        if dest.exits [inverse_direction [dir]] ~= uid then

          -- turn points into string, relative to where the room is
          local points = string.format ("%i,%i,%i,%i,%i,%i",
              x + arrow [1],
              y + arrow [2],
              x + arrow [3],
              y + arrow [4],
              x + arrow [5],
              y + arrow [6])

          -- draw arrow
          WindowPolygon(win, points,
                        exit_line_colour, miniwin.pen_solid, 1,
                        exit_line_colour, miniwin.brush_solid,
                        true, true)

        end -- one way

      end -- if we know of the room where it does

    end -- if we know what to do with this direction
  end -- for each exit


  if room.unknown then
    WindowCircleOp (win, miniwin.circle_rectangle, left, top, right, bottom,
                    config.UNKNOWN_ROOM_COLOUR.colour, miniwin.pen_dot, 1,  --  dotted single pixel pen
                    -1, miniwin.brush_null)  -- opaque, no brush
  else
    WindowCircleOp (win, miniwin.circle_rectangle, left, top, right, bottom,
                    0, miniwin.pen_null, 0,  -- no pen
                    room.fillcolour, room.fillbrush)  -- brush

    WindowCircleOp (win, miniwin.circle_rectangle, left, top, right, bottom,
                    room.bordercolour, room.borderpen, room.borderpenwidth,  -- pen
                    -1, miniwin.brush_null)  -- opaque, no brush
  end -- if


  -- show up and down in case we can't get a line in

  if room.exits.u then  -- line at top
    WindowLine (win, left, top, left + ROOM_SIZE, top, config.EXIT_COLOUR_UP_DOWN.colour, miniwin.pen_solid, 1)
  end -- if
  if room.exits.d then  -- line at bottom
    WindowLine (win, left, bottom, left + ROOM_SIZE, bottom, config.EXIT_COLOUR_UP_DOWN.colour, miniwin.pen_solid, 1)
  end -- if
  if room.exits ['in'] then  -- line at right
    WindowLine (win, left + ROOM_SIZE, top, left + ROOM_SIZE, bottom, config.EXIT_COLOUR_IN_OUT.colour, miniwin.pen_solid, 1)
  end -- if
  if room.exits.out then  -- line at left
    WindowLine (win, left, top, left, bottom, config.EXIT_COLOUR_IN_OUT.colour, miniwin.pen_solid , 1)
  end -- if

  speedwalks [uid] = path  -- so we know how to get here

  WindowAddHotspot(win, uid,
                 left, top, right, bottom,       -- rectangle
                 "mapper.mouseover_room",        -- mouseover
                 "mapper.cancelmouseover_room",  -- cancelmouseover
                 "",  -- mousedown
                 "",  -- cancelmousedown
                 "mapper.mouseup_room",  -- mouseup
                 room.hovermessage,
                 miniwin.cursor_hand, 0)  -- hand cursor

  WindowScrollwheelHandler (win, uid, "mapper.zoom_map")

end -- draw_room

local function changed_room (uid)

  hyperlink_paths = nil  -- those hyperlinks are meaningless now
  speedwalks = {}  -- old speedwalks are irrelevant

  if current_speedwalk then

    if uid ~= expected_room then
      local exp = rooms [expected_room]
      if not exp then
        exp = get_room (expected_room) or { name = expected_room }
      end -- if
      local here = rooms [uid]
      if not here then
        here = get_room (uid) or { name = uid }
      end -- if
      exp = expected_room
      here = uid
      maperror (string.format ("Speedwalk failed! Expected to be in '%s' but ended up in '%s'.", exp or "<none>", here))
      cancel_speedwalk ()
    else
      if #current_speedwalk > 0 then
        local dir = table.remove (current_speedwalk, 1)
        SetStatus ("Walking " .. (expand_direction [dir.dir] or dir.dir) ..
                   " to " .. walk_to_room_name ..
                   ". Speedwalks to go: " .. #current_speedwalk + 1)
        expected_room = dir.uid
        if config.DELAY.time > 0 then
          if GetOption ("enable_timers") ~= 1 then
            maperror ("WARNING! Timers not enabled. Speedwalking will not work properly.")
          end -- if timers disabled
          DoAfter (config.DELAY.time, dir.dir)
        else
          Send (dir.dir)
        end -- if
      else
        last_hyperlink_uid = nil
        last_speedwalk_uid = nil
        if show_completed then
          mapprint ("Speedwalk completed.")
        end -- if wanted
        cancel_speedwalk ()
      end -- if any left
    end -- if expected room or not
  end -- if have a current speedwalk

end -- changed_room

local function draw_zone_exit (exit)

  local x, y = exit.x, exit.y
  local offset = ROOM_SIZE

  -- draw circle around us
  WindowCircleOp (win, miniwin.circle_ellipse,
                  x - offset, y - offset,
                  x + offset, y + offset,
                  ColourNameToRGB "cornflowerblue",  -- pen colour
                  miniwin.pen_solid, -- solid pen
                  3, -- pen width
                  0, -- brush colour
                  miniwin.brush_null) -- null brush

  WindowCircleOp (win, miniwin.circle_ellipse,
                  x - offset, y - offset,
                  x + offset, y + offset,
                  ColourNameToRGB "cyan",  -- pen colour
                  miniwin.pen_solid, -- solid pen
                  1, -- pen width
                  0, -- brush colour
                  miniwin.brush_null) -- null brush

end --  draw_zone_exit


----------------------------------------------------------------------------------
--  EXPOSED FUNCTIONS
----------------------------------------------------------------------------------

-- can we find another room right now?

function check_we_can_find ()

  if not check_connected () then
    return
  end -- if

 if not current_room then
    mapprint ("I don't know where you are right now - try: LOOK")
    return false
  end -- if

  if current_speedwalk then
    mapprint ("No point doing this while you are speedwalking.")
    return false
  end -- if

  return true
end -- check_we_can_find

-- see: http://www.gammon.com.au/forum/?id=7306&page=2
-- Thanks to Ked.

-- uid is starting room
-- f returns true (or a "reason" string) if we want to store this one, and true,true if
--   we have done searching (ie. all wanted rooms found)

function find_paths (uid, f)

  local function make_particle (curr_loc, prev_path)
    local prev_path = prev_path or {}
    return {current_room=curr_loc, path=prev_path}
  end

  local depth = 0
  local count = 0
  local done = false
  local found, reason
  local explored_rooms, particles = {}, {}

  -- this is where we collect found paths
  -- the table is keyed by destination, with paths as values
  local paths = {}

  -- create particle for the initial room
  table.insert (particles, make_particle (uid))

  while (not done) and #particles > 0 and depth < config.SCAN.depth do

    -- create a new generation of particles
    new_generation = {}
    depth = depth + 1

    SetStatus (string.format ("Scanning: %i/%i depth (%i rooms)", depth, config.SCAN.depth, count))

    -- process each active particle
    for i, part in ipairs (particles) do

      count = count + 1

      if not rooms [part.current_room] then
        rooms [part.current_room] = get_room (part.current_room)
      end -- if not in memory yet

      -- if room doesn't exist, forget it
      if rooms [part.current_room] then

        -- get a list of exits from the current room
        exits = rooms [part.current_room].exits

        -- create one new particle for each exit
        for dir, dest in pairs(exits) do

          -- if we've been in this room before, drop it
          if not explored_rooms[dest] then
            explored_rooms[dest] = true
            rooms [dest] = supplied_get_room (dest)  -- make sure this room in table
            if rooms [dest] then
              new_path = copytable.deep (part.path)
              table.insert(new_path, { dir = dir, uid = dest } )

              -- if this room is in the list of destinations then save its path
              found, done = f (dest)
              if found then
                paths[dest] = { path = new_path, reason = found }
              end -- found one!

              -- make a new particle in the new room
              table.insert(new_generation, make_particle(dest, new_path))
            end -- if room exists
          end -- not explored this room
          if done then
            break
          end

        end  -- for each exit

      end -- if room exists

      if done then
        break
      end
    end  -- for each particle

    particles = new_generation
  end   -- while more particles

  SetStatus "Ready"
  return paths, count, depth
end -- function find_paths

-- draw our map starting at room: uid

function draw (uid)

  if not uid then
    maperror "Cannot draw map right now, I don't know where you are - try: LOOK"
    return
  end -- if

  if current_room and current_room ~= uid then
    changed_room (uid)
  end -- if

  current_room = uid -- remember where we are

  -- timing
  local start_time = utils.timer ()

  -- start with initial room
  rooms = { [uid] = get_room (uid) }

  -- lookup current room
  local room = rooms [uid]

  room = room or { name = "<Unknown room>", area = "<Unknown area>" }
  last_visited [uid] = os.time ()

  current_area = room.area

  -- we are recreating the window so any mouse-over is not valid any more
  if WindowInfo (win, 19) and WindowInfo (win, 19) ~= "" then
    if type (room_cancelmouseover) == "function" then
      room_cancelmouseover (WindowInfo (win, 19), 0)  -- cancelled mouse over
    end -- if
  end -- have a hotspot

  WindowDeleteAllHotspots (win)

  WindowCreate (win,
                 windowinfo.window_left,
                 windowinfo.window_top,
                 config.WINDOW.width,
                 config.WINDOW.height,
                 windowinfo.window_mode,   -- top right
                 windowinfo.window_flags,
                 config.BACKGROUND_COLOUR.colour)

  -- let them move it around
  movewindow.add_drag_handler (win, 0, 0, 0, font_height + 4)

  -- for zooming
  WindowAddHotspot(win,
                   "zzz_zoom",
                   0, 0, 0, 0,
                   "", "", "", "", "",
                   "",  -- hint
                   miniwin.cursor_arrow, 0)

  WindowScrollwheelHandler (win, "zzz_zoom", "mapper.zoom_map")

  -- set up for initial room, in middle
  drawn, drawn_coords, rooms_to_be_drawn, speedwalks, plan_to_draw, area_exits = {}, {}, {}, {}, {}, {}
  depth = 0

  -- insert initial room
  table.insert (rooms_to_be_drawn, add_another_room (uid, {}, config.WINDOW.width / 2, config.WINDOW.height / 2))

  while #rooms_to_be_drawn > 0 and depth < config.SCAN.depth do
    local old_generation = rooms_to_be_drawn
    rooms_to_be_drawn = {}  -- new generation
    for i, part in ipairs (old_generation) do
      draw_room (part.uid, part.path, part.x, part.y)
    end -- for each existing room
    depth = depth + 1
  end -- while all rooms_to_be_drawn

  for area, zone_exit in pairs (area_exits) do
    draw_zone_exit (zone_exit)
  end -- for

  local room_name = room.name
  local name_width = WindowTextWidth (win, FONT_ID, room_name, true)
  local add_dots = false

  -- truncate name if too long
  while name_width > (config.WINDOW.width - 10) do
    -- get rid of last word
    local s = string.match (" " .. room_name .. "...", "(%s%S*)$")
    if not s or #s == 0 then break end
    room_name = room_name:sub (1, - (#s - 2))  -- except the last 3 dots but add the space
    name_width = WindowTextWidth (win, FONT_ID, room_name .. " ...", true)
    add_dots = true
  end -- while

  if add_dots then
    room_name = room_name .. " ..."
  end -- if

  -- room name

  draw_text_box (win, FONT_ID,
                 (config.WINDOW.width - WindowTextWidth (win, FONT_ID, room_name, true)) / 2,   -- left
                 3,    -- top
                 room_name, true,             -- what to draw, utf8
                 config.ROOM_NAME_TEXT.colour,   -- text colour
                 config.ROOM_NAME_FILL.colour,   -- fill colour
                 config.ROOM_NAME_BORDER.colour)     -- border colour

  -- area name

  local areaname = room.area

  if areaname then
    draw_text_box (win, FONT_ID,
                   (config.WINDOW.width - WindowTextWidth (win, FONT_ID, areaname, true)) / 2,   -- left
                   config.WINDOW.height - 6 - font_height,    -- top
                   areaname, true,              -- what to draw, utf8
                   config.AREA_NAME_TEXT.colour,   -- text colour
                   config.AREA_NAME_FILL.colour,   -- fill colour
                   config.AREA_NAME_BORDER.colour)     -- border colour
  end -- if area known

  -- configure?

  if draw_configure_box then
    draw_configuration ()
  else

    local x = 5
    local y = config.WINDOW.height - 6 - font_height
    local width = draw_text_box (win, FONT_ID,
                   x,   -- left
                   y,   -- top (ie. at bottom)
                   "*", true,                   -- what to draw, utf8
                   config.AREA_NAME_TEXT.colour,   -- text colour
                   config.AREA_NAME_FILL.colour,   -- fill colour
                   config.AREA_NAME_BORDER.colour)     -- border colour

    WindowAddHotspot(win, "<configure>",
                   x, y, x + width, y + font_height,   -- rectangle
                   "",  -- mouseover
                   "",  -- cancelmouseover
                   "",  -- mousedown
                   "",  -- cancelmousedown
                   "mapper.mouseup_configure",  -- mouseup
                   "Click to configure map",
                   miniwin.cursor_hand, 0)  -- hand cursor
  end -- if

  if type (show_help) == "function" then
    local x = config.WINDOW.width - WindowTextWidth (win, FONT_ID, "?", true) - 5
    local y = config.WINDOW.height - 6 - font_height
    local width = draw_text_box (win, FONT_ID,
                   x,   -- left
                   y,   -- top (ie. at bottom)
                   "?", true,                   -- what to draw, utf8
                   config.AREA_NAME_TEXT.colour,   -- text colour
                   config.AREA_NAME_FILL.colour,   -- fill colour
                   config.AREA_NAME_BORDER.colour)     -- border colour

    WindowAddHotspot(win, "<help>",
                   x, y, x + width, y + font_height,   -- rectangle
                   "",  -- mouseover
                   "",  -- cancelmouseover
                   "",  -- mousedown
                   "",  -- cancelmousedown
                   "mapper.show_help",  -- mouseup
                   "Click for help",
                   miniwin.cursor_hand, 0)  -- hand cursor
  end -- if

  -- 3D box around whole thing

  draw_3d_box (win, 0, 0, config.WINDOW.width, config.WINDOW.height)

  -- make sure window visible
  WindowShow (win, not hidden)

  last_drawn = uid  -- last room number we drew (for zooming)

  local end_time = utils.timer ()

  -- timing stuff
  if timing then
    local count= 0
    for k in pairs (drawn) do
      count = count + 1
    end
    print (string.format ("Time to draw %i rooms = %0.3f seconds, search depth = %i", count, end_time - start_time, depth))

    total_times_drawn = total_times_drawn + 1
    total_time_taken = total_time_taken + end_time - start_time

    print (string.format ("Total times map drawn = %i, average time to draw = %0.3f seconds",
                          total_times_drawn,
                          total_time_taken / total_times_drawn))
  end -- if

end -- draw

local credits = {
  "MUSHclient mapper",
  string.format ("Version %0.1f", VERSION),
  "Written by Nick Gammon",
  WorldName (),
  GetInfo (3),
  }

-- call once to initialize the mapper
function init (t)

  -- make copy of colours, sizes etc.
  config = t.config
  assert (type (config) == "table", "No 'config' table supplied to mapper.")

  supplied_get_room = t.get_room
  assert (type (supplied_get_room) == "function", "No 'get_room' function supplied to mapper.")

  show_help = t.show_help     -- "help" function
  room_click = t.room_click   -- RH mouse-click function
  room_mouseover = t.room_mouseover -- mouse-over function
  room_cancelmouseover = t.room_cancelmouseover -- cancel mouse-over function
  timing = t.timing           -- true for timing info
  show_completed = t.show_completed  -- true to show "Speedwalk completed." message
  show_other_areas = t.show_other_areas  -- true to show other areas
  show_up_down = t.show_up_down        -- true to show up or down
  show_area_exits = t.show_area_exits  -- true to show area exits
  speedwalk_prefix = t.speedwalk_prefix  -- how to speedwalk (prefix)

  -- force some config defaults if not supplied
  for k, v in pairs (default_config) do
    config [k] = config [k] or v
  end -- for

  win = GetPluginID () .. "_mapper"

  WindowCreate (win, 0, 0, 0, 0, 0, 0, 0)

  -- add the fonts
  WindowFont (win, FONT_ID, config.FONT.name, config.FONT.size)
  WindowFont (win, FONT_ID_UL, config.FONT.name, config.FONT.size, false, false, true)

  -- see how high it is
  font_height = WindowFontInfo (win, FONT_ID, 1)  -- height

  -- find where window was last time
  windowinfo = movewindow.install (win, miniwin.pos_center_right)

  -- calculate box sizes, arrows, connecting lines etc.
  build_room_info ()

  WindowCreate (win,
                 windowinfo.window_left,
                 windowinfo.window_top,
                 config.WINDOW.width,
                 config.WINDOW.height,
                 windowinfo.window_mode,   -- top right
                 windowinfo.window_flags,
                 config.BACKGROUND_COLOUR.colour)

  -- let them move it around
  movewindow.add_drag_handler (win, 0, 0, 0, font_height + 4)

  local top = (config.WINDOW.height - #credits * font_height) /2

  for _, v in ipairs (credits) do
    local width = WindowTextWidth (win, FONT_ID, v, true)
    local left = (config.WINDOW.width - width) / 2
    WindowText   (win, FONT_ID, v, left, top, 0, 0, config.ROOM_COLOUR.colour, true)
    top = top + font_height
  end -- for

  draw_3d_box (win, 0, 0, config.WINDOW.width, config.WINDOW.height)

  WindowShow (win, true)

end -- init

function zoom_in ()
  if last_drawn and ROOM_SIZE < 40 then
    ROOM_SIZE = ROOM_SIZE + 2
    DISTANCE_TO_NEXT_ROOM = DISTANCE_TO_NEXT_ROOM + 2
    build_room_info ()
    draw (last_drawn)
  end -- if
end -- zoom_in


function zoom_out ()
  if last_drawn and ROOM_SIZE > 4 then
    ROOM_SIZE = ROOM_SIZE - 2
    DISTANCE_TO_NEXT_ROOM = DISTANCE_TO_NEXT_ROOM - 2
    build_room_info ()
    draw (last_drawn)
  end -- if
end -- zoom_out

function mapprint (...)
  local old_note_colour = GetNoteColourFore ()
  SetNoteColourFore(config.MAPPER_NOTE_COLOUR.colour)
  print (...)
  SetNoteColourFore (old_note_colour)
end -- mapprint

function maperror (...)
  local old_note_colour = GetNoteColourFore ()
  SetNoteColourFore(ColourNameToRGB "red")
  print (...)
  SetNoteColourFore (old_note_colour)
end -- maperror

function show ()
  WindowShow (win, true)
  hidden = false
end -- show

function hide ()
  WindowShow (win, false)
  hidden = true
end -- hide

function save_state ()
  movewindow.save_state (win)
end -- save_state


-- generic room finder

-- f (uid) is a function which returns: found, done
--    found is not nil if uid is a wanted room - if it is a string it is the reason it matched (eg. shop)
--    done is true if we know there is nothing else to search for (eg. all rooms found)

-- show_uid is true if you want the room uid to be displayed

-- expected_count is the number we expect to find (eg. the number found on a database)

-- if 'walk' is true, we walk to the first match rather than displaying hyperlinks

-- if fcb is a function, it is called back after displaying each line

function find (f, show_uid, expected_count, walk, fcb)

  if not check_we_can_find () then
    return
  end -- if

  if fcb then
    assert (type (fcb) == "function")
  end -- if

  local start_time = utils.timer ()
  local paths, count, depth = find_paths (current_room, f)
  local end_time = utils.timer ()

  local t = {}
  local found_count = 0
  for k in pairs (paths) do
    table.insert (t, k)
    found_count = found_count + 1
  end -- for

  -- timing stuff
  if timing then
    print (string.format ("Time to search %i rooms = %0.3f seconds, search depth = %i",
                          count, end_time - start_time, depth))
  end -- if

  if found_count == 0 then
    mapprint ("No matches.")
    return
  end -- if

  if found_count == 1 and walk then
    uid, item = next (paths, nil)
    mapprint ("Walking to:", rooms [uid].name)
    start_speedwalk (item.path)
    return
  end -- if walking wanted

  -- sort so closest ones are first
  table.sort (t, function (a, b) return #paths [a].path < #paths [b].path end )

  hyperlink_paths = {}

  for _, uid in ipairs (t) do
    local room = rooms [uid] -- ought to exist or wouldn't be in table

    assert (room, "Room " .. uid .. " is not in rooms table.")

    if current_room == uid then
      mapprint (room.name, "is the room you are in")
    else
      local distance = #paths [uid].path .. " room"
      if #paths [uid].path > 1 then
        distance = distance .. "s"
      end -- if
      distance = distance .. " away"

      local room_name = room.name
      if show_uid then
        room_name = room_name .. " (" .. uid .. ")"
      end -- if

      -- in case the same UID shows up later, it is only valid from the same room
      local hash = utils.tohex (utils.md5 (tostring (current_room) .. "<-->" .. tostring (uid)))

      Hyperlink ("!!" .. GetPluginID () .. ":mapper.do_hyperlink(" .. hash .. ")",
                room_name, "Click to speedwalk there (" .. distance .. ")", "", "", false)
      local info = ""
      if type (paths [uid].reason) == "string" and paths [uid].reason ~= "" then
        info = " [" .. paths [uid].reason .. "]"
      end -- if
      mapprint (" - " .. distance .. info) -- new line

      -- callback to display extra stuff (like find context, room description)
      if fcb then
        fcb (uid)
      end -- if callback
      hyperlink_paths [hash] = paths [uid].path
    end -- if
  end -- for each room

  if expected_count and found_count < expected_count then
    local diff = expected_count - found_count
    local were, matches = "were", "matches"
    if diff == 1 then
      were, matches = "was", "match"
    end -- if
    mapprint ("There", were, diff, matches,
              "which I could not find a path to within",
              config.SCAN.depth, "rooms.")
  end -- if

end -- map_find_things

-- executed when the mapper draws a hyperlink to a room

function do_hyperlink (hash)

  if not check_connected () then
    return
  end -- if

  if not hyperlink_paths or not hyperlink_paths [hash] then
    mapprint ("Hyperlink is no longer valid, as you have moved.")
    return
  end -- if

  local path = hyperlink_paths [hash]
  if #path > 0 then
    last_hyperlink_uid = path [#path].uid
  end -- if
  start_speedwalk (path)

end -- do_hyperlink

-- build a speedwalk from a path into a string

function build_speedwalk (path)

 -- build speedwalk string (collect identical directions)
  local tspeed = {}
  for _, dir in ipairs (path) do
    local n = #tspeed
    if n == 0 then
      table.insert (tspeed, { dir = dir.dir, count = 1 })
    else
      if tspeed [n].dir == dir.dir then
        tspeed [n].count = tspeed [n].count + 1
      else
        table.insert (tspeed, { dir = dir.dir, count = 1 })
      end -- if different direction
    end -- if
  end -- for

  if #tspeed == 0 then
    return
  end -- nowhere to go (current room?)

  -- now build string like: 2n3e4(sw)
  local s = ""

  for _, dir in ipairs (tspeed) do
    if dir.count > 1 then
      s = s .. dir.count
    end -- if
    if #dir.dir == 1 then
      s = s .. dir.dir
    else
      s = s .. "(" .. dir.dir .. ")"
    end -- if
    s = s .. " "
  end -- if

  return s

end -- build_speedwalk

-- start a speedwalk to a path

function start_speedwalk (path)

  if not check_connected () then
    return
  end -- if

  if current_speedwalk and #current_speedwalk > 0 then
    mapprint ("You are already speedwalking! (Ctrl + LH-click on any room to cancel)")
    return
  end -- if

  current_speedwalk = path

  if current_speedwalk then
    if #current_speedwalk > 0 then
      last_speedwalk_uid = current_speedwalk [#current_speedwalk].uid

      -- fast speedwalk: just send # 4s 3e  etc.
      if type (speedwalk_prefix) == "string" and speedwalk_prefix ~= "" then
        local s = speedwalk_prefix .. " " .. build_speedwalk (path)
        Execute (s)
        current_speedwalk = nil
        return
      end -- if

      local dir = table.remove (current_speedwalk, 1)
      local room = get_room (dir.uid)
      walk_to_room_name = room.name
      SetStatus ("Walking " .. (expand_direction [dir.dir] or dir.dir) ..
                 " to " .. walk_to_room_name ..
                 ". Speedwalks to go: " .. #current_speedwalk + 1)
      Send (dir.dir)
      expected_room = dir.uid
    else
      cancel_speedwalk ()
    end -- if any left
  end -- if

end -- start_speedwalk

-- cancel the current speedwalk

function cancel_speedwalk ()
  if current_speedwalk and #current_speedwalk > 0 then
    mapprint "Speedwalk cancelled."
  end -- if
  current_speedwalk = nil
  expected_room = nil
  hyperlink_paths = nil
  SetStatus ("Ready")
end -- cancel_speedwalk


-- ------------------------------------------------------------------
-- mouse-up handlers (need to be exposed)
-- these are for clicking on the map, or the configuration box
-- ------------------------------------------------------------------

function mouseup_room (flags, hotspot_id)
  local uid = hotspot_id

  if bit.band (flags, miniwin.hotspot_got_rh_mouse) ~= 0 then

    -- RH click

    if type (room_click) == "function" then
      room_click (uid, flags)
    end -- if

    return
  end -- if RH click

  -- here for LH click

   -- Control key down?
  if bit.band (flags, miniwin.hotspot_got_control) ~= 0 then
    cancel_speedwalk ()
    return
  end -- if ctrl-LH click

  start_speedwalk (speedwalks [uid])

end -- mouseup_room

-- ------------------------------------------------------------------
-- mouse-over handlers (need to be exposed)
-- these are for mousing over a room
-- ------------------------------------------------------------------

function mouseover_room (flags, hotspot_id)
  if type (room_mouseover) == "function" then
    room_mouseover (hotspot_id, flags)  -- moused over
  end -- if
end -- mouseover_room

function cancelmouseover_room (flags, hotspot_id)
  if type (room_cancelmouseover) == "function" then
    room_cancelmouseover (hotspot_id, flags)  -- cancled mouse over
  end -- if
end -- cancelmouseover_room

function mouseup_configure (flags, hotspot_id)
  draw_configure_box = true
  draw (current_room)
end -- mouseup_configure

function mouseup_close_configure (flags, hotspot_id)
  draw_configure_box = false
  draw (current_room)
end -- mouseup_player

function mouseup_change_colour (flags, hotspot_id)

  local which = string.match (hotspot_id, "^$colour:([%a%d_]+)$")
  if not which then
    return  -- strange ...
  end -- not found

  local newcolour = PickColour (config [which].colour)

  if newcolour == -1 then
    return
  end -- if dismissed

  config [which].colour = newcolour

  draw (current_room)
end -- mouseup_change_colour

function mouseup_change_font (flags, hotspot_id)

  local newfont =  utils.fontpicker (config.FONT.name, config.FONT.size, config.ROOM_NAME_TEXT.colour)

  if not newfont then
    return
  end -- if dismissed

  config.FONT.name = newfont.name

  if newfont.size > 12 then
    utils.msgbox ("Maximum allowed font size is 12 points.", "Font too large", "ok", "!", 1)
  else
    config.FONT.size = newfont.size
  end -- if

  config.ROOM_NAME_TEXT.colour = newfont.colour

  -- reload new font
  WindowFont (win, FONT_ID, config.FONT.name, config.FONT.size)
  WindowFont (win, FONT_ID_UL, config.FONT.name, config.FONT.size, false, false, true)

  -- see how high it is
  font_height = WindowFontInfo (win, FONT_ID, 1)  -- height

  draw (current_room)
end -- mouseup_change_font


function mouseup_change_width (flags, hotspot_id)

  local width = get_number_from_user ("Choose window width (200 to 1000 pixels)", "Width", config.WINDOW.width, 200, 1000)

  if not width then
    return
  end -- if dismissed

  config.WINDOW.width = width
  draw (current_room)
end -- mouseup_change_width

function mouseup_change_height (flags, hotspot_id)

  local height = get_number_from_user ("Choose window height (200 to 1000 pixels)", "Width", config.WINDOW.height, 200, 1000)

  if not height then
    return
  end -- if dismissed

  config.WINDOW.height = height
  draw (current_room)
end -- mouseup_change_height

function mouseup_change_depth (flags, hotspot_id)

  local depth = get_number_from_user ("Choose scan depth (3 to 100 rooms)", "Depth", config.SCAN.depth, 3, 100)

  if not depth then
    return
  end -- if dismissed

  config.SCAN.depth = depth
  draw (current_room)
end -- mouseup_change_depth

function mouseup_change_delay (flags, hotspot_id)

  local delay = get_number_from_user ("Choose speedwalk delay time (0 to 10 seconds)", "Delay in seconds", config.DELAY.time, 0, 10, true)

  if not delay then
    return
  end -- if dismissed

  config.DELAY.time = delay
  draw (current_room)
end -- mouseup_change_delay

function zoom_map (flags, hotspot_id)

  if bit.band (flags, 0x100) ~= 0 then
    zoom_out ()
  else
    zoom_in ()
  end -- if
end -- zoom_map


