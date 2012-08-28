-- movewindow.lua

--[[

Miniwindow drag-to-move functions.

Author: Nick Gammon
Date:   15th July 2009
Modified: 16th November 2010 to add preprocessing
Modified: 29th November 2010 by Fiendish to improve dragging offscreen

This module is intended to make it easier to add drag handlers for miniwindows.

It implements the following:

  -- find previous location
     nocheck: if true, don't check if offscreen  (boolean)
     friends: other windows to move with this one (table)
     preprocess: preprocessing for mousedown, mouseup etc. (table)
  
     Handler names for preprocess table:
  
       mousedown       
       mouseup         
       mouseover       
       cancelmouseover 
       cancelmousedown 
       dragmove        
       dragrelease  
    
     If any preprocess handler returns true (that is, neither nil nor false) then the default handler
     in this module is not used. (The miniwindow name is the third argument)
     
     function mousedown (flags, id, win)
        if bit.band (flags, miniwin.hotspot_got_rh_mouse) then
          -- do something different here
          return true
        end -- if
        
        return false   -- take normal movewindow behaviour
     end -- mousedown 

  windowinfo = movewindow.install (win, default_position, default_flags, nocheck, friends, preprocess)
  
  movewindow.add_drag_handler (win, left, top, right, bottom, cursor)   -- add a drag handler for the nominated rectangle
  
  movewindow.save_state (win)  -- saves the miniwindow location to the appropriate variables
  
It also installs a position-checker that moves the miniwindow into view after 5 seconds, in case
you resize the main world window, and the window is no longer visible. The 5 seconds are to give
the main world window's position and size time to stabilize. (Unless nocheck is true)


Example of use:

  require "movewindow"  -- pull in this module
  

  -- CREATE WINDOW in OnPluginInstall

  win = GetPluginID ()   -- miniwindow ID

  windowinfo = movewindow.install (win, miniwin.pos_center_right, 0)  -- default position / flags
  
  -- make miniwindow (use locations returned from last time we saved the state)
  -- note that the width and height are not part of the window position info, and can thus change as required
  
  WindowCreate (win, 
                windowinfo.window_left, 
                windowinfo.window_top, 
                WINDOW_WIDTH,    
                WINDOW_HEIGHT,  
                windowinfo.window_mode, 
                windowinfo.window_flags, 
                ColourNameToRGB "slategray") 

 
   
   -- INSTALL DRAG HANDLER when required (eg. when drawing stuff to window)
   --  in this case we use 0,0,0,0 as the rectangle (ie. the whole window)
   --  typically the height would be the size of the title bar
   
   movewindow.add_drag_handler (win, 0, 0, 0, 0, miniwin.cursor_both_arrow) 
 
   -- SAVE STATE in OnPluginSaveState
   
   movewindow.save_state (win)
   

   The module makes one global variable (table) when installed. This is named:
   
     mw_<window_id>_movewindow_info
   
   
   This contains handler functions (the table is an upvalue to the functions)
   
      "check_map_position"=function: 023D9368  -- the position checker   
      "dragmove"=function: 01AD1158            -- the dragmove handler   
      "dragrelease"=function: 023E4238         -- the dragrelease handler
      "margin"=20                              -- margin for dragging offscreen
      "mousedown"=function: 01AD1108           -- the mousedown handler
      "origx"=648                              -- used during dragging
      "origy"=39
      "startx"=88                              
      "starty"=8
      "win"="23c3c91af0a26790c625f5d1"         -- the supplied window ID
      "window_flags"=2                         -- flags (eg. 2, absolute position)
      "window_left"=652                        -- current left location           
      "window_mode"=0                          -- window mode                     
      "window_top"=31                          -- current top location            
    
  
    This table is returned from movewindow.install so you can find where to put the
    window the first time it is created.
    
--]]

movewindow = {}  -- table to hold functions like movewindow.install

-- make a mouse-down handler with the movement information as an upvalue

local function make_mousedown_handler (mwi)

  return function (flags, hotspot_id)
   
    local win = mwi.win
       
    -- see if other action wanted
    if mwi.preprocess.mousedown then
      if mwi.preprocess.mousedown (flags, hotspot_id, win) then
         return
      end -- if handled already
    end -- if handler
    
    -- find where mouse is so we can adjust window relative to mouse
    mwi.startx = WindowInfo (win, 14)
    mwi.starty = WindowInfo (win, 15)
    
    -- find where window is in case we drag it offscreen
    mwi.origx = WindowInfo (win, 10) 
    mwi.origy = WindowInfo (win, 11)
    
    -- find where the friends are relative to the window
    for i, v in ipairs (mwi.window_friends) do
        if v then
            mwi.window_friend_deltas [i] = 
               {
               WindowInfo (v, 10) - mwi.origx, 
               WindowInfo (v, 11) - mwi.origy
               }
        end -- if
    end -- for
        
  end -- mousedown

end -- make_mousedown_handler

-- make a mouse drag-move handler with the movement information as an upvalue

local function make_dragmove_handler (mwi)

  return function (flags, hotspot_id)
  
    local win = mwi.win
  
    -- see if other action wanted
    if mwi.preprocess.dragmove then
      if mwi.preprocess.dragmove (flags, hotspot_id, win) then
         return
      end -- if handled already
    end -- if handler
    
    -- find where it is now
    local posx, posy = WindowInfo (win, 17) - mwi.startx,
                       WindowInfo (win, 18) - mwi.starty
  
    -- change the mouse cursor shape appropriately
    if posx < 0 or 
       posx > GetInfo (281) - mwi.margin or
       posy < 0 or   -- don't drag title out of view
       posy > GetInfo (280) - mwi.margin then
      SetCursor (miniwin.cursor_x)   -- X cursor
    else
      SetCursor (miniwin.cursor_hand)    -- hand cursor
    end -- if
    
    if posx < 0 then
        posx = 0
    elseif posx > GetInfo (281) - mwi.margin then
        posx = GetInfo(281) - mwi.margin
    end
    if posy < 0 then
        posy = 0
    elseif posy > GetInfo(280) - mwi.margin then
        posy = GetInfo(280) - mwi.margin
    end
    
    -- move the window to the new location - offset by how far mouse was into window
    WindowPosition(win, posx, posy, 0, miniwin.create_absolute_location);
    
    -- move the friends if they still exist
    for i, v in ipairs(mwi.window_friends) do
        if v then
            WindowPosition (v, posx + mwi.window_friend_deltas [i] [1], 
                               posy + mwi.window_friend_deltas [i] [2], 
                               0, 
                               WindowInfo (v, 8))
        end -- if
    end -- for
        
    mwi.window_left = posx  -- remember for saving state
    mwi.window_top = posy
    mwi.window_mode = 0
    mwi.window_flags = miniwin.create_absolute_location   -- absolute position
    
  end -- dragmove

end -- make_dragmove_handler

-- make a mouse drag-release handler with the movement information as an upvalue

local function make_dragrelease_handler (mwi)

  return function (flags, hotspot_id)
  
    local win = mwi.win
  
    -- see if other action wanted
    if mwi.preprocess.dragrelease then
      if mwi.preprocess.dragrelease (flags, hotspot_id, win) then
         return
      end -- if handled already
    end -- if handler
    
    Repaint ()  -- update window location
    
  end -- dragrelease

end -- make_dragrelease_handler  
  
-- make other handler with the movement information as an upvalue

local function make_other_handler (mwi, name)

  return function (flags, hotspot_id)
       
    -- send to supplied handler
    if mwi.preprocess [name] then
       mwi.preprocess [name] (flags, hotspot_id, mwi.win)
    end -- if handler
        
  end -- other

end -- make_other_handler

-- make a mouse position-checking function with the movement information as an upvalue

local function make_check_map_position_handler (mwi)

  return function ()
   
    local win = mwi.win
    
    if not WindowInfo (win, 1) then
      ColourNote ("white", "red", "Error in make_check_map_position_handler: no window named: " .. win)
      return
    end -- no such window
    
     -- check miniwindow visible
      if mwi.window_left < 0 or 
         mwi.window_left > GetInfo (281) - mwi.margin or
         mwi.window_top < 0 or -- don't drag title out of view
         mwi.window_top > GetInfo (280) - mwi.margin then
           mwi.window_left, mwi.window_top = 0, 0  -- reset to center right
           mwi.window_mode = miniwin.pos_center_right
           mwi.window_flags = 0
      end -- if not visible
    
      WindowPosition (win, 
                      mwi.window_left, 
                      mwi.window_top, 
                      mwi.window_mode, 
                      mwi.window_flags)
    
  end -- check_map_position

end -- make_check_map_position_handler

-- call movewindow.install in OnPluginInstall to find the position of the window, before creating it
--  - it also creates the handler functions ready for use later

function movewindow.install (win, default_position, default_flags, nocheck, friends, preprocess, start_position)

  win = win or GetPluginID ()  -- default to current plugin ID
  
  assert (not string.match (win, "[^A-Za-z0-9_]"), "Invalid window name in movewindow.install: " .. win)
  
  default_position = default_position or miniwin.pos_center_right -- on right, center top/bottom
  default_flags = default_flags or 0
  
  -- set up handlers and where window should be shown (from saved state, if any)
  local movewindow_info = {
     win = win,   -- save window ID
     
     -- save current position in table (obtained from state file)
     window_left  = tonumber (GetVariable ("mw_" .. win .. "_windowx")) or (start_position and start_position.x) or 0,
     window_top   = tonumber (GetVariable ("mw_" .. win .. "_windowy")) or (start_position and start_position.y) or 0,     
     window_mode  = tonumber (GetVariable ("mw_" .. win .. "_windowmode")) or default_position,
     window_flags = tonumber (GetVariable ("mw_" .. win .. "_windowflags")) or default_flags,
     window_friends = friends or {},
     window_friend_deltas = {},
     margin = 20,  -- how close we can put to the edge of the window
     preprocess = preprocess or {},
    }

    -- check valid
    for k, v in pairs (movewindow_info.preprocess) do
       assert (type (v) == "function", "Handler '" .. k .. "' is not a function")
    end -- for
    
    -- handler to reposition window
    movewindow_info.check_map_position = make_check_map_position_handler (movewindow_info)  -- for startup
     
    -- mouse handlers
    movewindow_info.mousedown       = make_mousedown_handler   (movewindow_info)  
    movewindow_info.mouseup         = make_other_handler       (movewindow_info, "mouseup")
    movewindow_info.mouseover       = make_other_handler       (movewindow_info, "mouseover")
    movewindow_info.cancelmouseover = make_other_handler       (movewindow_info, "cancelmouseover")
    movewindow_info.cancelmousedown = make_other_handler       (movewindow_info, "cancelmousedown")
    movewindow_info.dragmove        = make_dragmove_handler    (movewindow_info)
    movewindow_info.dragrelease     = make_dragrelease_handler (movewindow_info)
 
  -- save table in global namespace
  _G ["mw_" .. win .. "_movewindow_info"] = movewindow_info
  
  
  -- give main world window time to stabilize its size and position                
  -- eg. this might be:  mw_23c3c91af0a26790c625f5d1_movewindow_info.check_map_position ()

  if not nocheck then  -- if wanted
    DoAfterSpecial (5, "mw_" .. win .. "_movewindow_info.check_map_position ()" , sendto.script)
  end -- if
  
  return movewindow_info  -- the caller might appreciate access to this table
end -- movewindow.install

-- call movewindow.add_drag_handler after creating the window, and after deleting hotspots where applicable
--   to add a drag hotspot

function movewindow.add_drag_handler (win, left, top, right, bottom, cursor)

  win = win or GetPluginID ()  -- default to current plugin ID

  -- the zz puts it under other hotspots on the drag area
  local hotspot_id = "zz_mw_" .. win .. "_movewindow_hotspot"
  
  if not WindowInfo (win, 1) then
    ColourNote ("white", "red", "Error in movewindow.add_drag_handler: no window named: " .. win)
    return
  end -- no such window
  
  -- make a hotspot
  WindowAddHotspot(win, hotspot_id,  
                   left or 0, top or 0, right or 0, bottom or 0,        -- rectangle
                   "mw_" .. win .. "_movewindow_info.mouseover",        -- MouseOver
                   "mw_" .. win .. "_movewindow_info.cancelmouseover",  -- CancelMouseOver
                   "mw_" .. win .. "_movewindow_info.mousedown",        -- MouseDown
                   "mw_" .. win .. "_movewindow_info.cancelmousedown",  -- CancelMouseDown
                   "mw_" .. win .. "_movewindow_info.mouseup",          -- MouseUp
                   "Drag to move window",  -- tooltip text
                   cursor or miniwin.cursor_hand, -- cursor
                   0)  -- flags
                   
  WindowDragHandler (win, hotspot_id, 
                     "mw_" .. win .. "_movewindow_info.dragmove", 
                     "mw_" .. win .. "_movewindow_info.dragrelease", 
                     0)   -- flags

end --  movewindow.add_drag_handler

-- call movewindow.save_state in OnPluginSaveState

function movewindow.save_state (win)

  win = win or GetPluginID ()  -- default to current plugin ID

  -- get movewindow variable from global namespace
  local mwi = _G ["mw_" .. win .. "_movewindow_info"]

  if not mwi then
    ColourNote ("white", "red", "Error in movewindow.save_state: no window movement info for: " .. win)
    return
  end -- no such window
  
  -- remember where the window was 
  SetVariable ("mw_" .. win .. "_windowx",      mwi.window_left)
  SetVariable ("mw_" .. win .. "_windowy",      mwi.window_top)
  SetVariable ("mw_" .. win .. "_windowmode",   mwi.window_mode)
  SetVariable ("mw_" .. win .. "_windowflags",  mwi.window_flags)
  
  -- use actual position, not where we happen to think it is, in case another plugin moves it
  -- suggested by Fiendish, 27 August 2012.
  if WindowInfo (win, 10) then
    SetVariable ("mw_" .. win .. "_windowx",    WindowInfo(win, 10))
  end
  if WindowInfo (win, 11) then
    SetVariable ("mw_" .. win .. "_windowy",    WindowInfo(win, 11))
  end
  

end -- movewindow.save_state  
