-- tabbed_window.lua

--[[

Author: Nick Gammon
Date:   7th February 2018

Generic tabbed miniwindow drawer.

Exposed functions:

  init (context)        -- call once (from OnPluginInstall), supply:
  
        context.win     -- window ID for this tabbed window
        context.font    -- table of font info (name, size, unicode)
        context.colours -- table of assorted colours (see below for examples)
        context.window  -- table (width and height) of the desired window size
        context.tab_filler  -- gap between tabs
        context.can_move    -- can window be dragged around?
        context.active_tab  -- the currently active tab
        context.tabs        -- table of tables - one per tab (name, handler)
       
  draw_window (context, which_tab)  -- draws the window with which_tab as the active tab
  hide_window (context)     -- hides this window 
  save_state  (context)     -- for saving the window position (call from OnPluginSaveState)
        
--]]


module (..., package.seeall)

require "movewindow"  -- load the movewindow.lua module
  
local default_context = { 

    -- window ID
    win = "tabbed_window_" .. GetUniqueID (),

    -- font for text
    font = {
        id = "fn",
        name = "Lucida Console",
        size = 10,  -- points
        unicode = false, -- Unicode?
      }, -- end of font info
      
    colours = {
      background    = ColourNameToRGB ("lightskyblue"),
      frame         = ColourNameToRGB ("mediumorchid"),
      title_bar     = ColourNameToRGB ("palegoldenrod"),
      title         = ColourNameToRGB ("mediumblue"),
      tab           = ColourNameToRGB ("green"),
      tab_text      = ColourNameToRGB ("white"),
      upper_line    = ColourNameToRGB ("lightgray"),
      lower_line    = ColourNameToRGB ("lightgray"),
      vertical_line = ColourNameToRGB ("lightgray"),
      }, -- end of colours
        
    -- miniwindow size
    window = {
      width = 300,
      height = 200,
      }, -- end of window info
          
    -- gap between text in tabs
    tab_filler = 10,
    
    -- can it be moved?
    can_move = true,
    
    -- which tab is active
    active_tab = 1,
    
    -- tabs
    -- table of tabs. For each one supply a table containing a name and a handler,
    --  eg. { name = "Circle", handler = DrawCircle },
    
    tabs = { },
    
    
} -- end of default_context


function init (context)

  -- make copy of colours, sizes etc.
  assert (type (context) == "table", "No 'context' table supplied to tabbed_window.")

  -- force some context defaults if not supplied
  for k, v in pairs (default_context) do
  
    -- some context items are tables so copy the table entries
    if type (v) == 'table' then
      if type (context [k]) ~= 'table' then
        assert (context [k] == nil, "tabbed window context item '" .. k .. "' should be a table or nil")
        context [k] =  { }  -- make table if required
      end -- not having a table there
      for k1, v1 in pairs (v) do
        context [k] [k1] = context [k] [k1] or v1
      end -- for each sub-table item
    else
      context [k] = context [k] or v
    end -- if table or not
  end -- for
  
  -- install the window movement handler, get back the window position
  context.windowinfo = movewindow.install (context.win, miniwin.pos_top_left)  -- default to top left
  
  -- save a bit of typing
  local windowinfo = context.windowinfo  
  local win        = context.win
  local font       = context.font
  local window     = context.window
  local colours    = context.colours
  
  -- make window so I can grab the font info
  WindowCreate (win, 
                 windowinfo.window_left, 
                 windowinfo.window_top, 
                 window.width, 
                 window.height,
                 windowinfo.window_mode,   
                 windowinfo.window_flags,
                 colours.background) 

  WindowFont (win, font.id, font.name, font.size, false, false, false, false, 0, 0)  -- normal  
  font.height = WindowFontInfo (win, font.id, 1)  -- height
 
  context.client_bottom = context.window.height - context.font.height - 8
  
end -- init

function save_state (context)
  -- save window current location for next time  
  movewindow.save_state (context.win)
end -- save_state

-- -------------------------------------------------------------------------------
-- draw_window - draw the tabbed window
-- -------------------------------------------------------------------------------
function draw_window (context, whichTab)

  context.active_tab = whichTab
  
  -- save a bit of typing
  local windowinfo    = context.windowinfo  
  local win           = context.win
  local font          = context.font
  local window        = context.window
  local colours       = context.colours
  local tabs          = context.tabs
  local client_bottom = context.client_bottom
  local tab_filler    = context.tab_filler
  
  -- clear window
  WindowRectOp (win, miniwin.rect_fill, 0, 0, 0, 0, colours.background)
  WindowDeleteAllHotspots (win)

    -- draw drag bar rectangle
  WindowRectOp (win, miniwin.rect_fill, 1, 1, 0, font.height + 2, colours.title_bar)

  -- add the drag handler so they can move the window around
  if context.can_move then
    movewindow.add_drag_handler (win, 0, 0, 0, font.height)
  end -- if can move the window

  local thisTab = tabs [whichTab]
  
  if not thisTab then
    ColourNote ("orange", "", "Tab " .. whichTab .. " does not exist")
    return
  end -- no such tab
  
  -- find title width so we can center it
  local title_width = WindowTextWidth (win, font.id, thisTab.name, font.unicode)
  
  -- draw title
  WindowText(win, font.id, thisTab.name, (window.width - title_width )/ 2 + 1, 1, 0, 0, colours.title, font.unicode)

  -- frame window
  WindowRectOp (win, miniwin.rect_frame, 0, 0, 0, 0, colours.frame)
  
  -- draw tabs
  
  local left = 1
  
  -- we have to make a function to handle each tabbed window, so its
  -- name must include the window ID
  local mouse_down_function_name = "tabbed_window_mouse_down_handler_" .. win
  
  -- the function will have the context as a closure
  getfenv () [mouse_down_function_name] = function (flags, hotspot_id)
      local whichTab = string.match (hotspot_id, "^hs(%d)$")
      if not whichTab then  -- tab unknown for some reason
        ColourNote ("orange", "", "Mousedown tab unknown " .. hotspot_id)
        return
      end -- if
      
      draw_window (context, tonumber (whichTab))
      
    end -- mouse_down_function_name
  
  for k, v in ipairs (tabs) do
    local tab_width =  WindowTextWidth (win, font.id, v.name, font.unicode)
    
    -- tab background
    WindowRectOp (win, miniwin.rect_fill, left, client_bottom, left + tab_width + tab_filler, window.height - 1, colours.tab)

    -- tab text
    WindowText(win, font.id, v.name, left + tab_filler / 2, client_bottom + 4, 0, 0, colours.tab_text, font.unicode)
    
    -- draw upper line if not active tab
    if k ~= whichTab then
      WindowLine(win, left, client_bottom + 2, left + tab_width + tab_filler, client_bottom + 2, 
                  colours.upper_line, miniwin.pen_solid, 2)
    end -- if not active
    
    -- draw lower line
    WindowLine(win, left, window.height - 1, left + tab_width + tab_filler, window.height - 1, 
                colours.lower_line, miniwin.pen_solid, 1)
    
    -- draw vertical lines
    WindowLine(win, left, client_bottom + 2, left, window.height - 1, 
                colours.vertical_line, miniwin.pen_solid, 1)
    WindowLine(win, left + tab_width + tab_filler, client_bottom + 2, left + tab_width + tab_filler, window.height - 1, 
                 colours.vertical_line, miniwin.pen_solid, 1)

    -- now add a hotspot for this tab
    WindowAddHotspot(win, "hs" .. k,  
                     left, client_bottom, left + tab_width + tab_filler, window.height - 1,   -- rectangle
                     "",   -- mouseover
                     "",   -- cancelmouseover
                     "tabbed_window." .. mouse_down_function_name,
                     "",   -- cancelmousedown
                     "",   -- mouseup
                     "Show " .. v.name .. " tab",  -- tooltip text
                     miniwin.cursor_hand, 0)  -- hand cursor
                         
    left = left + tab_width + tab_filler
    
  end -- for each tab
  
  -- call handler to draw rest of window
  local handler = thisTab.handler
  if handler then
    handler (win, 1, font.height + 2, window.width - 1, client_bottom, context)
  else
    ColourNote ("orange", "", "No tab handler for " .. thisTab.name)
  end -- if
  
  WindowShow (win, true) 
end -- draw_window

-- -------------------------------------------------------------------------------
-- hide_window - hide the tabbed window
-- -------------------------------------------------------------------------------
function hide_window (context)
  WindowShow (context.win, false) 
end -- hide_window