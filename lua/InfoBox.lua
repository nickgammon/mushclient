--[=[--
InfoBox.lua version 1.2
Encapsulates and enhances Infobar functionality into miniwindows; gauges and status text.
14-DEc-08

by: WillFa   (Spellbound on 3k.org:3000)
      portions borrowed or adapted from samples by Nick Gammon.

License: Free for public use in your MushClient plugins and scripts as long as credit for this module is
          attributed to WillFa, and Nick Gammon.

Assumes that the Sylfaen font is installed on the system. Change line 67 if it is not.

Requires: MushClient v. 4.37+ for complete functionality. (CloseWindow() uses DeleteWindow MC function.)
          MushClient v. 4.35+ for basic functionality. (WindowGradient calls)

Usage:
        require "InfoBox"
        MW = InfoBox:New()
        MW:AddBar("text", percent, "green", "red")
        MW:Update()

  Tutorial Demo can be found in the included InfoBox_Demo.xml plugin.

  Reference help for functions and values you will actually use can be obtained from the Doc() function.
    i.e.:
        InfoBox:Doc()
        InfoBox:Doc("AddBar")


  General Usage is more easily deduced by tprint(InfoBox), tprint(MW), tprint(MW.Bar) rather than reading this source.
    The metatables are setup in such a way that tprint will not recurse up the inheritance chain and spam you.
--]=]--

local world, colour_names = world, colour_names, _M
local os = {time = os.time}
local math, string, table, bit = math, string, table, bit
local tonumber, tostring, rawget, rawset, type, print = tonumber, tostring, rawget, rawset, type, print
local unpack, assert, require, getmetatable, setmetatable = unpack, assert, require, getmetatable, setmetatable
local pairs, ipairs, check = pairs, ipairs, check
local Global = _G
local capitalize, CalcShades, SplitRGB, idx, newidx
local CalcWindowHeight, strip_colours, sidewindows, Draw, DoFade, PrintText, ResizeOutput
local fontProps = {"fontID", "fontName", "fontSize", "fontBold", "fontItalic", "fontUnderline", "fontStrikeout", "fontCharset", "fontPitchAndFamily"}
local commas, CheckStyle, DoFade, PrintText
local matteHilight, matteShadow, Bar, ansiCodes, ansiColors, _Doc

module(...)

setmetatable ( _M, {__index = world, class = "module"})           -- find all the MushClient Functions

windowWidth = 240
windowHeight = 7                                -- this will change as more bars are added. It gives 5 pixels padding top and bottom.

columns = 0
rows = 0
axis = "rows"

backgroundColour = 0x808080
matteHilight = 0xb2b2b2
matteShadow = 0x4e4e4e

windowPosition = 7
windowFlags = 0

fontID = "fn"
fontName = "Sylfaen"
fontSize = 10
fontPadding = 2
fontBold = true
fontItalic = false
fontUnderline = false
fontStrikeout = false
fontCharset = 1
fontPitchAndFamily = 0

displaceOutput = false

ansiCodes = {"k", "r", "g", "y", "b", "m", "c", "w"}
ansiColors = {}                                                     -- Gets populated after capitalize() is defined.
customColourCodes = {}

-- Enumeration tables

textStyles = {  plain   = 0,
                matte   = 1,
                raised  = 2,
                sunken  = 4
            }

barStyles = {   textOnly        = 0,        -- Status line
                sunken          = 1,        -- Frames
                raised          = 2,
                raisedCap       = 2^2,      -- Caption is framed too.
                flat            = 2^3,
                glass           = 2^4,
                solid           = 2^5,      -- Fills
                gradientScale   = 2^6,      --   gradient midpoint is value/2. Fill shrinks
                gradientFixed   = 2^7,      --   gradient midpoint is always threshold.
                gradientShift   = 2^8,      --   Thermometer style; Fill remains 100%, gradient position shifts wth value
            }

captionPlacements = {   left            = 0,
                        innerLeft       = 1,
                        innerRight      = 2,
                        right           = 3,
                        center          = 4,
                        centerCell  = 5,
                    }

windowPositions = { NW  = 4,
                    N   = 5,
                    NE  = 6,
                    E   = 7,
                    SE  = 8,
                    S   = 9,
                    SW  = 10,
                    W   = 11,
                    }

-- End Enumerations


-- Bar Functions/Methods
Bar = { --Properties
        textStyle = 0,
        threshold = 30,
        textColour = 0x222222 ,
        "Caption",                  -- 1 = "Caption"    see InsertBars() for the tricky code that this uses.
        caption = "",
        "Value",                    -- 2 = "Value"      see InsertBars()
        value = 0,
        "GoodColour",               -- 3 = ...
        goodColour = 0x00DD00,
        "BadColour",                -- 4 = ...
        badColour = 0x0000DD,
        "AnchorRight",              -- 5 = ...
        anchorRight = false,
        "BarStyle",                 -- 6 = ...
        barStyle = 33,
        gaugeLeft = 5 ,
        captionPlacement = 1,
        padding = 3,
        cellPadding = 4,

        --End Properties
        }
setmetatable (Bar, {__index = _M, class = "bar"})

--[=[  Internal Function that writes captions ]=]--
function PrintText (self, vOffset, hOffset)
    --TraceOut (string.format("%s: Window %s Bar %i", "PrintText", self.windowName, self.id))
    local MatteText = strip_colours(self.caption)
    local width = WindowTextWidth (self.parent.windowName, self.fontID, MatteText)
    local side = 0
    if self.matteHilight == nil then
        self.matteHilight, self.matteShadow = self.backgroundColour,self.backgroundColour
        for i = 1,10 do
            self.matteHilight = AdjustColour(self.matteHilight,2)
            self.matteShadow = AdjustColour(self.matteShadow,3)
    end end
    vOffset = vOffset + self.fontPadding
    if self.captionPlacement == 0 then
        side = hOffset + self.gaugeLeft - WindowTextWidth ( self.parent.windowName, self.fontID, MatteText) -2
    elseif self.captionPlacement == 1 then
        side = hOffset + self.gaugeLeft + 5
    elseif self.captionPlacement == 2 then
        side = hOffset + self.gaugeLeft + self.gaugeWidth - width - 5
    elseif self.captionPlacement == 3 then
        side = hOffset + self.gaugeLeft + self.gaugeWidth + 2
    elseif self.captionPlacement == 4 then
        side = hOffset + self.gaugeLeft + ( (self.gaugeWidth-WindowTextWidth ( self.parent.windowName, self.fontID, MatteText) )/2)
    elseif self.captionPlacement == 5 then
        side = math.floor(hOffset + (self.cellWidth-WindowTextWidth ( self.parent.windowName, self.fontID, MatteText) ) / 2)
    end
    if self.barStyle == 0 then
        if self.captionPlacement > 3 then
            side = hOffset + math.floor( (self.cellWidth-WindowTextWidth ( self.parent.windowName, self.fontID, MatteText) ) / 2)
        elseif self.captionPlacement < 2 then
            side = hOffset + 5
        elseif self.captionPlacement > 1  then
            side = hOffset + self.parent.cellWidth - width - 5
    end end
    if self.textStyle > 0 then
        if (self.textStyle % 2) == 1 then                                                                                       -- Matted
            WindowText (self.parent.windowName, self.fontID, MatteText, side +1 , vOffset -1, 0, 0, self.backgroundColour)
                -- Upper Right Matte
            WindowText (self.parent.windowName, self.fontID, MatteText, side -1 , vOffset +1, 0, 0, self.backgroundColour)
                -- Lower Left Matte
          if self.textStyle == self.textStyles.matte then                                                                       -- Only Matted
            WindowText (self.parent.windowName, self.fontID, MatteText, side -1 , vOffset -1, 0, 0, self.backgroundColour)
                -- Top Left Matte
            WindowText (self.parent.windowName, self.fontID, MatteText, side +1 , vOffset +1, 0, 0, self.backgroundColour)
                -- Bottom Right Matte
          end
        end
        if bit.band(self.textStyle, self.parent.textStyles.raised) == self.textStyles.raised then                               -- Raised
            WindowText (self.parent.windowName, self.fontID, MatteText, side +1 , vOffset +1, 0, 0, self.matteShadow)
            -- Bottom Left Shadow
            WindowText (self.parent.windowName, self.fontID, MatteText, side -1 , vOffset -1, 0, 0, self.matteHilight )
            -- Top Left hilight
        end
        if bit.band(self.textStyle, self.textStyles.sunken) == self.textStyles.sunken then                                      -- Sunken
            WindowText (self.parent.windowName, self.fontID, MatteText, side -1 , vOffset -1, 0, 0, self.matteShadow)
            -- Top Left Shadow
            WindowText (self.parent.windowName, self.fontID, MatteText, side +1 , vOffset +1, 0, 0, self.matteHilight)
            -- Bottom Left Hilight
    end end
    if self.caption == MatteText then
        WindowText (self.parent.windowName, self.fontID, self.caption, side , vOffset, side + width, vOffset + self.fontHeight, self.textColour)      -- Text
    else
        ansiColors["~"] = self.textColour
        local customColours = self.customColourCodes or {}
        local Text = string.gsub (self.caption or "", "@@", "\0")  -- change @@ to 0x00
        if Text:sub (1, 1) ~= "@" then
          Text = "@~" .. Text
        end
        for colour, text in Text:gmatch ("@([%a~])([^@]+)") do
            text = text:gsub ("%z", "@") -- put any @ characters back
            if #text > 0 then
                side = side + WindowText (self.windowName, self.fontID, text, side, vOffset, hOffset + self.cellWidth, vOffset + self.fontHeight, customColours[colour] or ansiColors[colour] or ansiColors["~"])
    end end end
    return vOffset - self.fontPadding
end

--[=[  Internal Function that does bitmask checking ]=]--
function CheckStyle(self, style)
    --TraceOut (string.format("%s: Window %s Bar %i", "CheckStyle", self.windowName, self.id))
    if bit.band(self.barStyle, style) > 0 then
        return true
    else
        return false
end end

--[=[  Internal Function that draws the pretty boxes ]=]--
function Draw (self, vOffset, hOffset)
    --TraceOut (string.format("%s: Window %s Bar %i", "Draw", self.windowName, self.id))
    self.cellTop, self.cellLeft = vOffset , hOffset
    WindowRectOp (self.windowName , 2, self.cellLeft, self.cellTop - 1 , self.cellLeft + self.cellWidth , self.cellTop + self.cellHeight + self.padding - 2 , self.backgroundColour, nil)
    local Percent = self.value or 0
    local colour, fcolour
    if Percent < self.threshold then
        colour = self.badColour
    elseif Percent > 100 then
        Percent = 100
        for i = 1,15 do
            colour = AdjustColour(self.goodColour,4)                -- 20% more luminence
        end
    else
        colour = self.goodColour
    end
    if self.fade == true then
        colour = DoFade(self)
        fcolour = colour
    end
    local pixels = self.gaugeWidth * Percent / 100
    if self.barStyle > 0 then
        local bezel = {[true] = 10, [false] = 5}
        local frameLeft = self.cellLeft + self.gaugeLeft
        local frameTop =  self.cellTop + self.cellHeight - self.gaugeHeight
        local frameRight =  self.cellLeft + self.gaugeLeft + self.gaugeWidth
        local frameBottom = self.cellTop + self.cellHeight
        local topX, topY, bottomX, bottomY = frameLeft + 2, frameTop + 2 , self.cellLeft + self.gaugeLeft + pixels, frameBottom - 2
        local colorL, colorR = self.badColour, (fcolour or self.goodColour)
        local ftopX, ftopY, fbottomX, fbottomY = bottomX + 1, frameTop, frameRight, frameBottom  --fill region for fixed gradients
        if self.anchorRight == true then
            topX , topY         = frameRight - pixels , frameTop + 2
            bottomX, bottomY    = frameRight - 2, frameBottom - 2
            ftopX , ftopY       = frameLeft , frameTop
            fbottomX, fbottomY  = frameRight - pixels - 2, frameBottom
            colorL, colorR      = (fcolour or self.goodColour), self.badColour
        end
        if bit.band(barStyles.raisedCap, self.barStyle) == barStyles.raisedCap then
            if self.captionPlacement == 0 then
                frameLeft = self.cellLeft + 3
            elseif self.captionPlacement == 3 then
                frameRight = self.cellLeft + self.cellWidth -3
        end end
        if Percent >= 1  then
            if CheckStyle(self, self.barStyles.solid) then
                check (WindowRectOp (self.parent.windowName, 2, topX , topY , bottomX , bottomY , colour) )
            elseif CheckStyle(self, self.barStyles.gradientScale) then                                   -- scaled gradient fills
                check (WindowGradient (self.parent.windowName, topX, topY, bottomX, bottomY, colorL, colorR, 1 ) )
            elseif CheckStyle(self, self.barStyles.gradientFixed) then                                   -- fixed gradients.
                local padWidth, padFillColour = self.gaugeWidth * ( (math.abs(50-self.threshold) *2)/100), (fcolour or self.goodColour)
                if self.threshold > 50 then
                    padFillColour = self.badColour
                end
                check (WindowRectOp (self.parent.windowName, 2, topX , topY , bottomX, bottomY, padFillColour) )
                if  ( (self.anchorRight == false) and (self.threshold <= 50) ) or
                    ( (self.anchorRight == true)  and (self.threshold  > 50) ) then         --need more colorR than colorL
                        check (WindowGradient (self.parent.windowName, self.cellLeft + self.gaugeLeft, frameTop, frameRight - padWidth, bottomY, colorL, colorR, 1 ) )
                elseif  ( (self.anchorRight == true)  and (self.threshold <= 50) ) or
                        ( (self.anchorRight == false) and (self.threshold  > 50) ) then     --need more colorL than colorR
                        check (WindowGradient (self.parent.windowName, self.cellLeft + self.gaugeLeft + padWidth, topY, frameRight -2, bottomY, colorL, colorR, 1 ) )
                end
                check (WindowRectOp (self.parent.windowName, 2, ftopX , ftopY , fbottomX , fbottomY -1 , self.backgroundColour) )
            elseif CheckStyle(self, self.barStyles.gradientShift) then                       -- Thermometer gradients.
                local midpointX = bottomX
                local pixel = self.gaugeWidth / 100
                local gradientWidth =  math.min(30, (100 - self.value) * pixel, self.value * pixel)
                if self.anchorRight == true then
                    colorL, colorR = (fcolour or self.goodColour), self.badColour
                    midpointX = topX
                end
                check (WindowRectOp (self.parent.windowName, 2, topX , topY , bottomX , bottomY, (fcolour or self.goodColour) ) )
                check (WindowRectOp (self.parent.windowName, 2, ftopX, ftopY , fbottomX, bottomY, self.badColour) )
                check (WindowGradient (self.parent.windowName, midpointX - gradientWidth, topY, midpointX + gradientWidth, bottomY, colorR, colorL, 1 ) )
                check (WindowRectOp (self.parent.windowName, 2, self.cellLeft + 1, self.cellTop, self.cellLeft + self.gaugeLeft, self.cellTop + self.cellHeight , self.backgroundColour) )
                check (WindowRectOp (self.parent.windowName, 2, frameRight, frameTop, -2, frameBottom, self.backgroundColour) )
            end
            if  CheckStyle(self, self.barStyles.glass) then                                      -- glass frame
                local glassWidth, dtX,dtY, dbX, dbY = pixels, topX, topY,  bottomX, bottomY
                if CheckStyle(self, self.barStyles.gradientShift) then
                    glassWidth = self.gaugeWidth
                    dtX, dtY, dbX, dbY = frameLeft + 2, frameTop + 1, frameRight - 2, frameBottom -1
                end
                check (WindowCreate ( self.parent.windowName .. "glass", 0,0, glassWidth, self.cellHeight - 2, 7, 0, 0x171717) )
                check (WindowGradient(self.parent.windowName .. "glass", 0,0,  0, self.cellHeight * .50, 0x606060, 0xaaaaaa, 2) )
                check (WindowGradient(self.parent.windowName .. "glass", 0,self.cellHeight * .51,  0, self.cellHeight -2, 0x7c7c7c, 0x393939, 2) )
                check (WindowImageFromWindow ( self.parent.windowName, self.parent.windowName .. "glass", self.parent.windowName .. "glass") )
                check (WindowBlendImage ( self.parent.windowName, self.parent.windowName .. "glass", dtX, dtY,  dbX, dbY, 21, .45) )
            elseif CheckStyle(self, barStyles.raised + barStyles.raisedCap + barStyles.sunken) then      -- 3D frame rectangle
                check (WindowRectOp (self.parent.windowName, 5, topX , topY , bottomX, bottomY, bezel[not CheckStyle(self, barStyles.sunken + barStyles.glass)] ,0x100F) )
        end end
        if bit.band( self.barStyle , 31 ) ~= 0 then                         -- frame rectangle on border
            local pen = {[true]=1,[false]=5}
            check (WindowRectOp (self.parent.windowName, pen[CheckStyle(self, self.barStyles.flat)], frameLeft, frameTop, frameRight, frameBottom, bezel[not CheckStyle(self, barStyles.raised + barStyles.raisedCap)], 15+0x1000) )
    end end
    PrintText (self, self.cellTop, self.cellLeft)
    if type(self.button) == 'table' then
        WindowAddHotspot(self.windowName, self.id, self.cellLeft, self.cellTop, self.cellLeft + self.cellWidth, self.cellTop + self.cellHeight,
            self.button.mouseOver or "", self.button.cancelMouseOver or "", self.button.mouseDown or "", self.button.cancelMouseDown or "",
            self.button.mouseUp or "", self.button.tooltipText or "", self.button.cursor or 0, 0 )

    end
    return self.cellTop + self.cellHeight + self.padding
end

--[=[  Internal Function that calculates different shades of colors ]=]--
function DoFade(self)
    --TraceOut (string.format("%s: Window %s Bar %i", "DoFade", self.windowName, self.id))
    local color
    if self.shades == nil or (self.Bar.shades and self.goodColour ~= self.Bar.goodColour and self.badColour ~= self.Bar.badColour) then
        self.shades = CalcShades(self, math.ceil(100 - self.threshold)/5)
    end
    if self.value >= self.threshold then
        color = self.shades[math.ceil( (101-self.value)/5 )] or self.shades[#self.shades]
    else
        color = self.badColour
    end
    return color
end


--[=[     See :Doc( "Fade" )  --]=]--
function Bar:Fade (bool)
	assert(bool and type(bool)=="boolean", "Boolean expected. Got " .. tostring( bool ) .. "(" .. type(bool) .. ")")
	self.fade = bool
    self.shades = nil
	if bool then
		DoFade(self)
	end
end


--[=[     See :Doc( "TextColour" )  --]=]--
function Bar:TextColour (x)
    --TraceOut (string.format("%s: Window %s Bar %i", "TextColour", self.windowName, self.id))
    local n = tonumber(x) or tonumber(x,16) or ColourNameToRGB(x)
    assert(n and n~= -1,'Bad value passed. Acceptable examples: "red", 255, 0x0000ff, "#FF0000"  - Got '.. tostring(x) .. '('.. type(x) ..')')
    self.textColour = n
end

--[=[     See :Doc( "ColourText" )  --]=]--
function Bar:ColourText(val, thresh, good, bad, neut)
    --TraceOut (string.format("%s: Window %s Bar %i", "ColourText", self.windowName, self.id))
    local n = tonumber(thresh)
    if n then
        if val == thresh then
            self:TextColour(neut or good)
        elseif val < thresh then
            self:TextColour(bad)
        else
            self:TextColour(good)
end end end

--[=[     See :Doc( "GoodColour" )  --]=]--
function Bar:GoodColour (x)
    --TraceOut (string.format("%s: Window %s Bar %i", "GoodColour", self.windowName, self.id))
    local n = tonumber(x) or ColourNameToRGB(x)
    assert(n and n~= -1,'Bad value passed. Acceptable examples: "red", 255, 0x0000ff, "#FF0000"  - Got '.. tostring(x) .. '('.. type(x) ..')')
    self.goodColour = n
    self.shades = nil
end

--[=[     See :Doc( "BadColour" )  --]=]--
function Bar:BadColour (x)
    --TraceOut (string.format("%s: Window %s Bar %i", "BadColour", self.windowName, self.id))
    local n = tonumber(x) or ColourNameToRGB(x)
    assert(n and n~= -1,'Bad value passed. Acceptable examples: "red", 255, 0x0000ff, "#FF0000"  - Got '.. tostring(x) .. '('.. type(x) ..')')
    self.badColour = n
    self.shades = nil
end

--[=[     See :Doc( "AnchorRight" )  --]=]--
function Bar:AnchorRight (bool)
    --TraceOut (string.format("%s: Window %s Bar %i", "AnchorRight", self.windowName, self.id))
    if type(bool) == "boolean" then
        bool = bool
    else
        bool = false
    end
    self.anchorRight = bool
    if bool == false then
        self.captionPlacement = 1
    else
        self.captionPlacement = 2
end end

--[=[     See :Doc( "Value" )  --]=]--
function Bar:Value (val)
    --TraceOut (string.format("%s: Window %s Bar %i", "Value", self.windowName, self.id))
    self.value = tonumber(val) or self.value
    self.watchValue = nil
end

--[=[     See :Doc( "Caption" )  --]=]--
function Bar:Caption (txt)
    --TraceOut (string.format("%s: Window %s Bar %i", "Caption", self.windowName, self.id))
    self.watchCaption = nil
    if tonumber(txt) then
        self.caption = commas(txt)
    elseif txt == nil then
        self.caption = ""
    else
        self.caption = txt
    end
end

--[=[     See :Doc( "TextStyle" )  --]=]--
function Bar:TextStyle (x)
    --TraceOut (string.format("%s: Window %s Bar %i", "TextStyle", self.windowName, self.id))
    self.textStyle = tonumber(x) or 1
end

--[=[     See :Doc( "Threshold" )  --]=]--
function Bar:Threshold (x)
    --TraceOut (string.format("%s: Window %s Bar %i", "Threshold", self.windowName, self.id))
    self.threshold = tonumber(x) or 30
    self.shades = nil
end

--[=[     See :Doc( "BarStyle" )  --]=]--
function Bar:BarStyle (x)
    --TraceOut (string.format("%s: Window %s Bar %i", "BarStyle", self.windowName, self.id))
    assert ( tonumber(x) and tonumber(x) >= 0, "Invalid barstyle.")
    self.barStyle = tonumber(x)
end

--[=[     See :Doc( "Padding" )  --]=]--
function Bar:Padding (x)
    --TraceOut (string.format("%s: Window %s Bar %i", "Padding", self.windowName, self.id))
    assert( tonumber(x), "Number of pixels needed. Got " .. type(x) )
    local MiniWindow = self.parent or self
    self.padding = tonumber(x)
    MiniWindow:Resize()
end

--[=[     See :Doc( "WatchValue" )  --]=]--
function Bar:WatchValue (varName)
    --TraceOut (string.format("%s: Window %s Bar %i", "WatchValue", self.windowName, self.id))
    self.value = nil
    self.watchValue = varName
end

--[=[     See :Doc( "WatchCaption" )  --]=]--
function Bar:WatchCaption (varName)
    --TraceOut (string.format("%s: Window %s Bar %i", "WatchCaption", self.windowName, self.id))
    self.caption = nil
    self.watchCaption = varName
end

--[=[  Internal Function that hides in metatables ]=]--
local function idx (self, k)
    local varName, val = rawget (self, "watch" .. capitalize(k) ) or ""
    if (k == "value" or k == "caption" ) then
        for z in  varName:gmatch("([^%.%[%]]+)") do
            local parent = val or Global
            z = tonumber(z) or z
            val = parent[z]
        end
        return val
    else
        local Parent = getmetatable(self)["parent"]                 --parent is hidden in a metatable so it doesn't tprint. Hooray for aesthetically pleasing wastes of processor cycles.
        val = rawget (self, k)
        val = val or Parent[k]
        return val
end end

--[=[  Internal Function that hides in metatables ]=]--
local function newidx (self, k, v)
    if (k == "value" and rawget (self,watchValue) ) or (k == "caption" and rawget (self, watchCaption) ) then
        self["watch" .. capitalize(k)] = nil
    end
    rawset(self, k, v)
end


-- End Bar Functions


-- Global Functions

--[=[     See :Doc( "New" )  --]=]--
function _M:New (winName)
    --TraceOut (string.format("%s: Window %s", "New", winName))
    local mt = {
        __index =
        function (self, key)
            if key == "cellWidth" then
                return self.windowWidth / self.columns
            elseif key == "gaugeWidth" then
                return self.cellWidth - 10
            else
                return _M[key]
        end end ,
        __metatable = {class = "Box"}; }
    local q = {}
    setmetatable(q,mt)                                          -- Window table inherits the module
    q.Bars = {}
    q.Bar = {}
    for k,v in pairs(Bar) do q.Bar[k] = v end                   -- a separate instance of Bar so that...
    setmetatable(q.Bar, {parent = q, class = "Bar", __index=
        function (self, key)
            local mt = getmetatable(self)
            if key =="parent" then
                return mt.parent
            else
                return mt.parent[key]
        end end ,
        })                                                      -- Bar table instance inherits it's MiniWindow table instance
    q.windowName = winName or CreateGUID()                      -- Will create a guid if none passed in. Fine for plugin use.
    check (WindowCreate (q.windowName, 0, 0, 1, 1, q.windowPosition, q.windowFlags, q.backgroundColour) )
    check (WindowFont (q.windowName, q.fontID, q.fontName, q.fontSize, q.fontBold, q.fontItalic, q.fontUnderline, q.fontStrikeout, q.fontCharset, q.fontPitchAndFamily) )
    q.fontHeight = WindowFontInfo (q.windowName, q.fontID, 1)                                               -- Default font height
    q.Bar.cellHeight = q.fontHeight + q.Bar.cellPadding
    q.Bar.gaugeHeight = q.Bar.cellHeight                                                                        -- Default size of bars.
    return q
end

--[=[     See :Doc( "BackgroundColour" )  --]=]--
function _M:BackgroundColour (col)
    assert (col, "Nil passed. specify a colour.")
    local n = tonumber(col) or ColourNameToRGB(col)
    assert(n and n~= -1,'Bad value passed. Acceptable examples: "red", 255, 0x0000ff, "#FF0000"  - Got '.. tostring(x) .. '('.. type(x) ..')')
    self.backgroundColour = n
    self.matteHilight, self.matteShadow = self.backgroundColour,self.backgroundColour
    for i = 1,10 do
        self.matteHilight = AdjustColour(self.matteHilight,2)
        self.matteShadow = AdjustColour(self.matteShadow,3)
    end
end

--[=[     See :Doc( "Rows" )  --]=]--
function _M:Rows(num)
    assert(type(num)=="number" and num > 0, "How many rows? Need a positive number. Got " .. type(num) )
    assert(self.parent == nil, "Pass a MiniWindow")
    self.rows = num
    self:Resize()
end

--[=[     See :Doc( "Columns" )  --]=]--
function _M:Columns(num)
    assert(type(num)=="number" and num > 0, "How many columns? Need a positive number. Got " .. type(num) )
    assert(self.parent == nil, "Pass a MiniWindow")
    self.rows = math.ceil(#self.Bars/num)
    self.columns = num
    self:Resize()
end

--[=[     See :Doc( "AddBar" )  --]=]--
function _M:AddBar(...)
    --TraceOut (string.format("%s: Window %s", "AddBar", self.windowName))
    return self:InsertBar(#self.Bars + 1, ...)
end

--[=[     See :Doc( "InsertBar" )  --]=]--
function _M:InsertBar(index, ...)
    --TraceOut (string.format("%s: Window %s Bar %i", "InsertBar", self.windowName, index))
    assert(self ~= _M , "Pass a table created from ".. _NAME .. ":New(), not ".. _NAME .. " itself.")
    assert(self.parent == nil, "Attempt to add a Bar to a Bar. Use table from ".. _NAME .. ":New() instead.")
    assert(index, "Bars index must be passed.")
    local MiniWindow = self.parent or self
--[[--  Expected possible values:
            1= Caption,
            2= Value,
            3= GoodColor,
            4= BadColor,
            5= AnchoredRight,     (whether the 0 value is on the left or right side of the gauge.)
            6= BarStyle           (enumerated in InfoBox.barStyles      )               --]]--
    local args={...}
    table.insert (self.Bars, index, {})                                                 -- Add a table for new bar.
    setmetatable(self.Bars[index],{__metatable={parent = self.Bar}, __index = idx, __newindex= newidx})
    for i = index, #self.Bars do
        self.Bars[i].id = i                                                             -- a way to find our index in the future
    end
    for k,v in ipairs(args) do                                                          -- update with optional parameters passed in
        MiniWindow.Bar[ MiniWindow.Bar[k] ] (MiniWindow.Bars[index], v)                 -- This line of code is why MW.Bar[1] = "Caption". Bar[1] thru Bar[5] define which function the optional parameters call. Neat, hunh?
    end
    self:Resize()
    return self.Bars[index]
end

--[=[     See :Doc( "RemoveBar" )  --]=]--
function _M:RemoveBar(index)
    --TraceOut (string.format("%s: Window %s", "RemoveBar", self.windowName))
    assert(self ~= _M , "Call from a table created from ".. _NAME .. ":New(), not ".. _NAME .. " itself.")
    assert(self.parent == nil, "Attempt to remove a Bar from a Bar. Use table from ".. _NAME .. ":New() instead.")
    assert(index, "Bars index must be passed.")
    table.remove (self.Bars, index)
    local otheraxis = {columns = "rows", rows = "columns"}                              -- Shrink the grid if appropriate
    if (self[self.axis] -1 ) * self[ otheraxis[self.axis] ] >= #self.Bars then
        self[self.axis] = self[self.axis] -1
        self:Resize()
    end
    for i = index, #self.Bars do
        self.Bars[i].id = i
end end

--[=[     See :Doc( "Update" )  --]=]--
function _M:Update (force)
    --TraceOut (string.format("%s: Window %s", "Update", self.windowName))
    assert(self ~= _M , "Pass a table called from ".. _NAME .. ":New(), not ".. _NAME .. " itself.")
            --Note: It is possible to pass any table instance into this. i.e. foo, foo.Bar, foo.Bars[i]
            --It may be bad style, but it intentionally will work. foo.Bars[i] still updates the entire window, not just its region.
    if os.time() > (self.lastUpdated or 0) or force then
        local MiniWindow = self.parent or self
        check (WindowCreate (self.windowName, 0, 0, self.windowWidth, self.windowHeight, self.windowPosition, self.windowFlags, GetInfo(278)) )
        check (WindowRectOp (self.windowName, 2, 2, 2, -2, -2, self.backgroundColour) )  -- "erase" miniwindw
        local vertical,horizontal, curRow, curCol, t = 6, 0, 1, 1
        for _, curBar in ipairs(self.Bars) do
            local finishY = vertical + curBar.cellHeight + curBar.padding
            if curRow > self.rows or finishY > self.windowHeight then
                vertical = 6
                curCol = curCol + 1
                curRow = 1
                if curCol > self.columns then               --Maybe Fonts are distorting the grid
                    self:Resize()                           -- Resize/CalcWindowHeight will fix it
                    self:Update()                           -- and we need to redraw.
                    return                                  -- and stop execution of the outdated update.
            end end
            horizontal =  (curCol -1) * self.cellWidth
            vertical = Draw(curBar, vertical, horizontal)
            curRow=curRow+1
        end
        check (WindowRectOp (self.windowName, 5, 0, 1, -1, -2, 10, 15) )                -- 3D border effect on canvas
        WindowRectOp (self.windowName, 2, self.windowWidth - 2 , 0, 0, 0, GetInfo(278), nil)
        WindowShow (self.windowName, true)
        self.lastUpdated = os.time()
        return true
    else
        return false
end end

--[=[     See :Doc( "WindowPosition" )  --]=]--
function _M:WindowPosition(pos)
    assert(type(pos) == "number" and (pos >= 4 and pos <=11), "Invalid window position passed. Use " .. _NAME .. ".windowPositions[ <N,S,E,W,NE...> ] if unsure.")
    assert(self ~= _M and self.parent == nil, "Pass a table created from ".. _NAME .. ":New(), not ".. _NAME .. " itself.")
    world.WindowPosition(self.windowName, 0,0,pos, self.windowFlags)
    if ( (self.windowPosition == 5 or self.windowPosition == 9) and (pos ~= 5 and pos ~=9) or
         (self.windowPosition ~= 5 and self.windowPosition ~= 9) and (pos == 5 or pos ==9) ) then
        self.displaceOutput = (pos == 5 or pos == 9)
        self.columns, self.rows = self.rows, self.columns
        self.axis = "columns"
    end
    self.windowPosition = pos
    self:Resize()
end

--[=[     See :Doc( "ResizeOutput" )  --]=]--
function _M:ResizeOutput()
    local maxWidth, maxHeight = 0,0
    local left, top, width, height = GetInfo(272), GetInfo(273), GetInfo(274), GetInfo(275)
    local bOff, bCol, bWid, oCol, oSty = GetInfo(276), GetInfo(282), GetInfo(277), GetInfo(278), GetInfo(279)
    local t={}
    for _, v in ipairs(WindowList() or {}) do
        local tmpPos, shown = WindowInfo(v, 7), WindowInfo(v,5)
        if shown then
            t[tmpPos] = (t[tmpPos] or 0) + 1
    end end
            local function sum (...)
                local args, res= {...}, 0
                for _,n in ipairs(args) do
                    if type(t[n]) == "number" then res = (res + t[n]) end
                end
                return res
            end
    if self.displaceOutput then
        if self.windowPosition == 9 or self.windowPosition == 10 then
            if sum(9,10) == 1 then
                height = -self.windowHeight - 8
            else
                height = -math.max(math.abs(height), self.windowHeight)
            end
        elseif self.windowPosition == 11 then
            if t[11] == 1 then
                left = self.windowWidth
            else
                left = math.max ( left, self.windowWidth)
            end
        elseif self.windowPosition == 4 or self.windowPosition == 5 then
            if sum(4,5) == 1 then
                top = self.windowHeight
            else
                top = math.max( top , self.windowHeight)
            end
        else
            if sum(6,7,8) == 1 then
                width = -1 * self.windowWidth
            else
                width = -1 * math.max( math.abs(width) , self.windowWidth )
    end end end
    if sum(11)      == 0    then left   = 4     end
    if sum(9,10)    == 0    then height = 0     end
    if sum(5, 4)    == 0    then top    = 0     end
    if sum(6, 7, 8) == 0    then width  = 0     end
    TextRectangle(left, top, width, height,  bOff, bCol, bWid, oCol, oSty)
end

--[=[     See :Doc( "Resize" )  --]=]--
function _M:Resize()
    assert (self ~= _M , "Resize a window, not the module.")
    assert (not self.parent, "Resize() is for MiniWindows, not Bars.")
    local sharedWindow, winwidths = {}, 0
    local sidewindows = {}
    CalcWindowHeight(self)
    if self.windowPosition == 5 or self.windowPosition == 9 then
        local padptrn = "padS[EW]$"
        if self.windowPosition == 5 then padptrn = "padN[EW]$" end
        for _,v in ipairs(WindowList() ) do
            if v:find(padptrn) then
                WindowDelete(v)
            elseif WindowInfo(v,5) then
                local Position = WindowInfo(v, 7)
                sidewindows[Position] = sidewindows[Position] or {topmost = 100000, bottommost = 0, leftmost = 100000, rightmost = 0, topwidth = 0, leftheight = 0, rightheight = 0}
                sidewindows[Position].topmost = math.min( sidewindows[Position].topmost, WindowInfo(v, 11) )
                sidewindows[Position].bottommost = math.max( sidewindows[Position].bottommost, WindowInfo(v, 13) )
                sidewindows[Position].leftmost = math.min(sidewindows[Position].leftmost, WindowInfo(v, 10) )
                sidewindows[Position].rightmost = math.max(sidewindows[Position].rightmost, WindowInfo(v, 12 ) )
                if WindowInfo(v,11) == sidewindows[Position].topmost then
                    sidewindows[Position].topwidth =  WindowInfo(v,3)
                end
                if Position == self.windowPosition then
                    sharedWindow[#sharedWindow +1] = v
                    sidewindows[Position].leftmost = math.min(sidewindows[Position].leftmost, WindowInfo(v, 10) )
                    sidewindows[Position].rightmost = math.max(sidewindows[Position].rightmost, WindowInfo(v, 12) )
                    if sidewindows[Position].leftmost == WindowInfo(v, 10) then
                        sidewindows[Position].leftheight = WindowInfo ( v, 4 )
                    end
                    if sidewindows[Position].rightmost == WindowInfo(v, 12) then
                        sidewindows[Position].rightheight = WindowInfo ( v, 4 )
        end end end end

--[[        Prevent overlapping Auto-centered windows.
            Windows will be arranged:
                LttttR
                L    R
                L    R
                BBBBBB                                                  --]]

        if self.windowPosition == 5 then
            if sidewindows[11] and sidewindows[11].topmost < sidewindows[5].bottommost then
                sidewindows[4] = sidewindows[4] or {topwidth = 0 }
                sidewindows[4].topwidth = math.max(sidewindows[4].topwidth, sidewindows[11].topwidth )
                WindowCreate(self.windowName .. "padNW", 0,0,sidewindows[11].topwidth,0, 4, 0, 0)
                WindowShow(self.windowName .. "padNW")
            end

            if sidewindows[7] and sidewindows[7].topmost < sidewindows[5].bottommost then
                sidewindows[6] = sidewindows[6] or {topwidth = 0 }
                sidewindows[6].topwidth = math.max(sidewindows[6].topwidth, sidewindows[7].topwidth )
                WindowCreate(self.windowName .. "padNE", 0,0,sidewindows[7].topwidth,0, 6, 0, 0)
                WindowShow(self.windowName .. "padNE")
            end
        elseif self.windowPosition == 9 then
            if sidewindows[11] and sidewindows[11].bottommost > GetInfo(280) - sidewindows[9].leftheight then
                WindowCreate(self.windowName .. "padSW", 0,0,0,sidewindows[9].leftheight, 10, 0, 0)
                WindowShow(self.windowName .. "padSW")
            end
            if sidewindows[7] and sidewindows[7].bottommost > GetInfo(280) - sidewindows[9].rightheight then
                WindowCreate(self.windowName .. "padSE", 0,0,0, sidewindows[9].rightheight, 8, 0, 0)
                WindowShow(self.windowName .. "padSE")
        end end
        sidewindows[self.windowPosition -1] = sidewindows[self.windowPosition -1] or {topwidth = 0}
        sidewindows[self.windowPosition +1] = sidewindows[self.windowPosition +1] or {topwidth = 0}
        self.windowWidth = math.floor( (GetInfo(281) - sidewindows[self.windowPosition -1].topwidth - sidewindows[self.windowPosition +1].topwidth ) / #sharedWindow)
        if WindowInfo(self.windowName, 6) then   -- we're hidden and a sharedwindow isn't an infobox that will resize itself.
            for _,p in ipairs({4,6,8,10}) do
                sidewindows[p] = sidewindows[p] or {topwidth = 0}
            end
            winwidths = (sidewindows[self.windowPosition - 1].topwidth) + (sidewindows[self.windowPosition + 1].topwidth)
            for _,v in ipairs(sharedWindow) do
                if v ~= self.windowName then
                    winwidths = winwidths + WindowInfo(v,3)
            end end
            self.windowWidth = GetInfo(281) - winwidths
    end end
    self:ResizeOutput()
end

--[=[     See :Doc( "CloseWindow" )  --]=]--
function _M:CloseWindow ()
    local wins = {"", "glass", "NE", "NW", "SE", "SW"}
    for i = 1,5 do
        WindowDelete(self.windowName .. wins[i])
        self:ResizeOutput()
end end

--[=[     See :Doc( "Font" )  --]=]--
function _M:Font(...)
    --[=[   Optional parameters:
    --          Name, string
    --          Size, number
    --          Bold, boolean
    --          Italic, boolean
    --          Underline, boolean
    --          Strikeout, boolean
    --          Charset, number
    --          PitchAndFamily, number
    --]=]
    assert(self ~= _M , "Pass a table created from ".. _NAME .. ":New(), not ".. _NAME .. " itself.")
    local fontParams = {}
    local gaugeRatio = (self.gaugeHeight or self.Bar.gaugeHeight) / (self.cellHeight or self.Bar.cellHeight)
    for k,v in ipairs(fontProps) do fontParams[k] = self[v] end
    local MiniWindow = self.parent or self
    local args = {...}
    if args[1] == nil then
        if self == MiniWindow then
            local defaultFont = {}
            for _,f in ipairs(fontProps) do self[f] = nil ; defaultFont[#defaultFont +1] = self[f] end
            check( WindowFont (self.windowName, unpack(defaultFont) ) )
            self.fontHeight = WindowFontInfo (self.windowName, self.fontID, 1)
            self.Bar.cellHeight = self.fontHeight + self.Bar.cellPadding
            self.Bar.gaugeHeight = self.Bar.cellHeight * gaugeRatio
        else
            for _,f in ipairs(fontProps) do self[f] = nil end
            self.fontHeight = nil
            self.cellHeight     = nil
            self.gaugeHeight = self.cellHeight * gaugeRatio
        end
    else
        for k,v in ipairs(args) do
            fontParams[k + 1] = v
            self[fontProps[k + 1] ] = v
        end
        self.fontName, self.fontSize = name, fontParams[1]
        if self == MiniWindow then
            MiniWindow.windowHeight = _M.windowHeight
            check(WindowFont(MiniWindow.windowName, unpack(fontParams) ) )
            MiniWindow.Bar.cellHeight = WindowFontInfo (MiniWindow.windowName, MiniWindow.fontID, 1) + MiniWindow.Bar.cellPadding
        else
            fontParams[1]= "f" .. self.id
            check(WindowFont(self.windowName, unpack(fontParams) ) )
            for k,v in ipairs(fontProps) do if fontParams[k] ~= self.parent[v] then self[v] = fontParams[k] end end
            self.fontHeight = WindowFontInfo (self.windowName, self.fontID, 1)
            self.cellHeight = self.fontHeight + self.cellPadding
            self.gaugeHeight = self.cellHeight * gaugeRatio
    end end
    MiniWindow:Resize()
end

--[=[     See :Doc( "ReopenWindow" )  --]=]--
function _M:ReopenWindow()
    local MiniWindow = self.parent or self
    check(WindowCreate(MiniWindow.windowName, 0,0,MiniWindow.windowWidth,MiniWindow.windowHeight, MiniWindow.windowPosition, MiniWindow.windowFlags, MiniWindow.backgroundColour) )
    WindowShow(MiniWindow.windowName)
    MiniWindow:Resize()
    local font = {}
    for k,v in ipairs (fontProps) do font[k] = MiniWindow[v] end
    WindowFont (MiniWindow.windowName, unpack(font))
    for _, Bar in ipairs(MiniWindow.Bars) do
        if Bar.fontID ~= MiniWindow.fontID then
            for k,v in ipairs (fontProps) do font[k] = Bar[v] end
            WindowFont (Bar.windowName, unpack(font))
    end end
    MiniWindow:Update()
end

--[=[     See :Doc( "CaptionPlacement" )  --]=]--
function _M:CaptionPlacement (x)
    assert((self.id and x>=0 and x<=5) or (not self.parent and x ==nil),"call MiniWindow:CaptionPlacement() or Bar:CaptionPlacement(x), x must be a value in the ".. _NAME .. ".captionPlacements table. Got ".. tostring(x) .. " (" .. type(x) ..")")
    local m, MiniWindow = 5, self.parent or self
    if self.parent then self.captionPlacement = x end
    local startCol, endCol, barID
    if self.id then                                             -- Hit just the column
        startCol = math.modf(self.id/self.rows)
        endCol   = startCol
    else                                                        -- Hit em all
        startCol = 0
        endCol   = MiniWindow.columns -1
    end
    for curCol = startCol, endCol do
        local maxCaplength, capsPlaced = 5, {}
        barID = (curCol * self.rows) + 1
        while curCol == math.modf( ( barID -1)/self.rows ) and barID <= #MiniWindow.Bars do
            local cBar = MiniWindow.Bars[ barID ]
            if cBar.barStyle ~= 0 then
                maxCaplength = math.max(maxCaplength,WindowTextWidth(MiniWindow.windowName, cBar.fontID, strip_colours(cBar.caption), false)+5 )
                capsPlaced[cBar.captionPlacement] = true
            end
            barID = barID +1
        end
        barID = (curCol * self.rows) + 1
        while curCol == math.modf( (barID - 1)/self.rows ) and barID <= #MiniWindow.Bars do
            local cBar = MiniWindow.Bars[ barID ]
            if capsPlaced[0] and not capsPlaced[3] then
                cBar.gaugeLeft = maxCaplength
                cBar.gaugeWidth = MiniWindow.cellWidth - cBar.gaugeLeft -8
            elseif not capsPlaced[0] and capsPlaced[3] then
                cBar.gaugeLeft = nil
                cBar.gaugeWidth = MiniWindow.cellWidth - maxCaplength -8
            elseif capsPlaced[0] and capsPlaced[3] then
                cBar.gaugeLeft = maxCaplength
                cBar.gaugeWidth = MiniWindow.cellWidth  - (2 * maxCaplength) - 2
            else
                cBar.gaugeLeft = nil
                cBar.gaugeWidth = nil
            end
            barID = barID + 1
end end end


--[[-- Friend Functions --]]--

--[=[  Internal Function  ]=]--
function CalcWindowHeight (self)
    local MiniWindow = self.parent or self
    local tmpHeights = {(MiniWindow.Bar.cellHeight + MiniWindow.Bar.padding) * MiniWindow.rows, 0}
    local modOffset = 0
    local otheraxis = {columns = "rows", rows = "columns"}
    if MiniWindow[ otheraxis[MiniWindow.axis] ] == 0 then
        MiniWindow[ otheraxis[MiniWindow.axis] ] = 1
    end
    if #MiniWindow.Bars > (MiniWindow.rows * MiniWindow.columns) then                               -- Is the grid big enough?
        repeat
            MiniWindow[MiniWindow.axis] = MiniWindow[MiniWindow.axis] + 1
        until #MiniWindow.Bars <= MiniWindow.columns * MiniWindow.rows
    end

    MiniWindow:CaptionPlacement()
    for _,tmpBar in ipairs(MiniWindow.Bars or {}) do                                                -- Calculate the single largest column size.
        if ( (tmpBar.id + modOffset) -1 ) % MiniWindow.rows == 0 then
            tmpHeights[ #tmpHeights + 1 ] = tmpBar.cellHeight + tmpBar.padding
        else
            tmpHeights[#tmpHeights] = tmpHeights[#tmpHeights] + tmpBar.cellHeight + tmpBar.padding
        end
        if (tmpBar.cellHeight + tmpBar.padding) > (MiniWindow.Bar.cellHeight + MiniWindow.Bar.padding) and
            #MiniWindow.Bars < (MiniWindow.rows*MiniWindow.columns) then
                modOffset = -(tmpBar.id % MiniWindow.rows)
        end
    end
    MiniWindow.windowHeight = math.max(unpack(tmpHeights) ) + _M.windowHeight
    check (WindowCreate (MiniWindow.windowName, 0, 0, MiniWindow.windowWidth, MiniWindow.windowHeight, MiniWindow.windowPosition, MiniWindow.windowFlags, MiniWindow.backgroundColour) )
    WindowShow(MiniWindow.windowName)
end

--[=[  Internal Function used in DoFade  ]=]--
function SplitRGB (Colour)
    local _, r,g,b
    if type(Colour) == "number" or colour_names[Colour] then
            _,_, b,g,r = string.format("%06x", (colour_names[Colour] or Colour) ):find("(%x%x)(%x%x)(%x%x)")
    elseif (type(Colour) == "string")   then                                                --HTML Formatted string  "#RRGGBB" or something similar
            _,_, r,g,b = Colour:find("(%x%x)(%x%x)(%x%x)")
    end
    r,g,b = tonumber(r, 16), tonumber(g,16), tonumber(b,16)
    return r,g,b , string.format("#%02x%02x%02x", r,g,b):upper()
end

--[=[  Internal Function used in DoFade  ]=]--
function CalcShades(self, shades) -- the number of shades including start and end
    local c, step, Cs  = {}, {}, {}
    local r1, g1, b1 = SplitRGB(self.goodColour)
    local r2, g2, b2 = SplitRGB(self.badColour)
    step.r = (r1-r2)/shades
    step.g = (g1-g2)/shades
    step.b = (b1-b2)/shades
    for i = 0, shades do
        Cs[i+1] = tonumber(string.format("%02x%02x%02x",  math.floor(b1 - (step.b * i) ) % 256 , math.floor(g1 - (step.g * i) ) % 256 , math.floor(r1 - (step.r * i) ) % 256 ),16)
    end
    Cs[#Cs] = self.badColour
    return Cs, step, step.r, step.g, step.b
end

--[=[  Internal Function used for color codes ]=]--
function capitalize (s)
    return s:gsub("%a",string.upper, 1)
end -- capitalize

--[=[  Internal Function used in PrintText  ]=]--
function strip_colours (s)
    s=s or ""
    s = s:gsub ("@@", "\0")  -- change @@ to 0x00
    s = s:gsub ("@.([^@]*)", "%1")
    return (s:gsub ("%z", "@") ) -- put @ back
end

--[=[  Internal Function used in Caption, graciously borrowed from Nick Gammon  ]=]--
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

--[=[  Populates color codes tables  ]=]--
for i,v in ipairs(ansiCodes) do
    ansiColors[v] = GetNormalColour (i)
    ansiColors[capitalize(v)] = GetBoldColour (i)
end



--[=[  Documentation - MW:Doc() will be easier to read then the source of the table  ]=]--

_Doc = {
AddBar = [[
Applies To:  MiniWindow table
Prototype: myBar = MW:AddBar(caption, value, goodColour, badColour, anchorRight, barStyle)

Adds a new bar to the end of the [MiniWindow].Bars table and returns a reference to this table.

caption = string; if a number is passed, commas are added to the ciphers
value = number; the percentage of the bar that is filled. Overmax percentages are accepted, and goodColour is drawn 15% brighter.
goodColour = string|number; the string is checked against names of colours or HTML formatting, the long number value of a color is also accepted.
badColour = string|number; same as goodColour
anchorRight = boolean; if true, 100% is on the left, 0% on the right. Automatically sets caption to be innerRight if true.
barStyle = number; a bitmask of barStyles values that specify a frame and fill type.]],
CaptionPlacement = [[
Applies To: Miniwindow table, Bar
Prototype: Bar:CaptionPlacement(n) or MW:CaptionPlacement()

Sets where text is drawn in the bar and calculates gauge sizes as appropriate. If calling against the MiniWindow itself, do not pass a parameter in the function to recalculate all gauge sizes and placements.
Setting a value to the MiniWindow.Bar.captionPlacement value will change the default label placement.

n = number; a value of 0-5 which are enumerated in the captionPlacements table.]],
CloseWindow = [[
Applies To: Miniwindow
Prototype: MW:CloseWindow()

Intended to be called from OnPluginDisable or OnPluginClose. Deletes the MiniWindow from MushClient's list of miniwindows.
Call MW:ReopenWindow() in the OnPluginEnable function.
]], --']]
Columns = [[
Applies To: Bar
Prototype: Bar:Columns(n)

Sets the number of columns into which the miniwindow is divided.

n = number; The number of columns.]],
Font = [[
Applies To: MiniWindow, Bar
Prototype: MW:Font([fontName, fontSize, fontBold, fontItalic, fontUnderline, fontStrikeout, fontPitchAndFamily])

If no parameters are passed, deletes font settings on the specific object so it will reinherit from a parent object.
Parameters omitted will retrieve settings from those named values. Specified values set named value on object as well.

See MushClient documentation for "WindowFont" for information on parameters.]],
InsertBar = [[
Applies To: MiniWindow
Prototype: myBar = MW:InsertBar(n, caption, value, goodColour, badColour, anchorRight, barStyle)

Inserts a new bar at position n and returns a reference to this new bar table.

n = number; The 1-based index of the new bar.
caption = string; if a number is passed, commas are added to the ciphers
value = number; the percentage of the bar that is filled. Overmax percentages are accepted, and goodColour is drawn 15% brighter.
goodColour = string|number; the string is checked against names of colours or HTML formatting, the long number value of a color is also accepted.
badColour = string|number; same as goodColour
anchorRight = boolean; if true, 100% is on the left, 0% on the right. Automatically sets caption to be innerRight if true.
barStyle = number; a bitmask of barStyles values that specify a frame and fill type.]],
New = [[
Applies To: Module
Prototype: MW = ]] .. _NAME .. [[:New([name])

Creates a new miniwindow table for you to populate with bars. The returned table has a metatable with its __index value referencing ]] .. _NAME .. [[.

name = string; If specified, sets the name of the window returned by MushClient's WindowList function. Omitting this value generates a GUID for MushClient to use for the window name. This name is stored in the MW.windowName value.]], --']]
ReopenWindow = [[
Applies To: MiniWindow
Protoype: MW:ReopenWindow()

Intended to be used in the OnPluginEnabled callback so that fonts are reloaded after the miniwindow is deleted with WindowDelete() (i.e. OnPluginDisabled called MW:CloseWindow() (but not MW = nil) which deletes the window from WindowList(). ]],
RemoveBar = [[
Applies To: MiniWindow
Prototype: MW:RemoveBar(n)

Deletes the specified bar from the Bars table.

n = number; The index of the bar to delete. A bar stores its index in the Bar.id value for your convenience.]],
Resize = [[
Applies To: MiniWindow
Prototype: MW:Resize()

This is called internally when necessary, the only time you need to be aware of it is for windows positioned at the top-center or bottom-center of the screen since they grow to the available width. It is recommended to place a call in the OnPluginWorldOutputResized callback in this scenario.]],
ResizeOutput = [[
Applies To: MiniWindow
Prototype: MW:ResizeOutput()

Called internally when positioning miniwindows at the left-center, lower-left, or bottom-center positions. Ff you wish another window to resize the Mud's text area, call it manually after setting that Miniwindow's .displaceOutput value to true.]], --']]
Rows = [[
Applies To: MiniWindow
Prototype: MW:Rows(n)

Sets the number of rows in a miniwindow to a specific value. If the MW.axis value is equal to "rows" it may still grow when new bars exceed the available space.

n = number; A positive number of rows]],
Update = [[
Applies To: MiniWindow
Prototype: updated = MW:Update()

Draws or redraws the Miniwindow. This function will only redraw once a second to prevent excessive CPU utilization during speedwalk or lagbursts.

updated = boolean; True if the window was redrawn this call, false if it had been drawn already in the past second and not updated.]],
WindowPosition = [[
Applies To: MiniWindow
Prototype: MW:WindowPosition(n)

Sets the location for the MiniWindow.

n = number; a value between 4 and 11. Values are enumerated in the .windowPositions table.]],
axis = [[
Applies To: MiniWindow
ProtoType: MW.axis = <"rows"|"columns">

Determines which way the grid scales as new bars demand additional space. MW.windowHeight grows as new rows are added. MW.windowWidth remains the same as new columns divide that space into smaller pieces.]],
backgroundColour = [[
Applies To: MiniWindow
ProtoType: MW.backgroundColour = n

The colour of the miniwindow.

n = number; The number value of the colour desired. i.e. colour_names.indigo, not "indigo"]],
barStyles = [[
Applies To: All
Prototype: N/A

A table enumerating the possible barStyle fills and frames.

none      =  0;  no frame
sunken    =  1;  a raised bar in a recessed frame
raised    =  2;  a recessed bar in a raised frame
raisedCap =  4;  a recessed bar in a raised frame that fills the whole cell, regardless of captionPlacement
flat      =  8;  a single pixel frame, untouched gauge effect
glass     = 16;  recessed frame and a pretty visual effect on the bar

solid         =  32;  a plain color fill
gradientScale =  64; a gradient that goes from 0 to .value
gradientFixed = 128; gradient goes from 0 to 100, with backgroundColour covering .value +1 to 100
gradientShift = 256; The thermometer effect. Gradient that has its midpoint at .value.]],
captionPlacements = [[
Applies To: ALL
Prototype: N/A

A table enumerating values for the location of labels.

left = 0; Text is drawn to the left of the gauge.
innerLeft = 1; Text is drawn left justified close to the 0 value
innerRight = 2; Text is drawn right justified by the 100 value
right = 3; Text is drawn to the right of the gauge
center = 4; Text is drawn centered within the gauge.
centerCell = 5; Text is drawn centered within the entire bar.]],
columns = [[Applies To: MiniWindow
Prototype: MW.columns = n

The number of columns the MW is divided into. Setting this value directly bypasses validation in the Columns function.

n = number; Expected to be a positive, non-zero integer.]],
customColourCodes = [[
Applies to: MiniWindow, Bar
Prototype: Bar.customColourCodes = {a=val, [b=val2, ...]}

A table used for drawing different colours within the caption. See caption for more information.

a = char; a single character key identifying the attached value.
val = number; the number value of the desired colour. i.e. {i= colour_names.indigo}]],
displaceOutput = [[
Applies To: MiniWindow
Prototype:  MW.displaceOutput = bool

Set to true and call MW:ResizeOutput() if you want to shrink the area that mud text arrives in so your window doesn't float above it. MiniWindows placed at the left-center, lower-left, or bottom-center (W,SW,S positions) automatically set this value to true.]], --']]
fontBold = [[
Applies To: MiniWindow, Bar
Prototype:  Bar.fontBold = bool

This property can be used to specify the default value for the Font function, or be used to retrieve its last set value.]],
fontCharset = [[
Applies To: MiniWindow, Bar
Prototype:  Bar.fontCharset = bool

This property can be used to specify the default value for the Font function, or be used to retrieve its last set value.
See MushClient's WindowFont documentation for more information]], --']]
fontID = [[
Applies To: MiniWindow, Bar
Prototype:  foo = Bar.fontID

This property is used internally and really shouldn't be needed in your scripting.]], --']]
fontItalic = [[
Applies To: MiniWindow, Bar
Prototype:  Bar.fontItalic = bool

This property can be used to specify the default value for the Font function, or be used to retrieve its last set value.
See MushClient's WindowFont documentation for more information]], --']]
fontName = [[
Applies To: Module and MiniWindow, Bar
ProtoType: ]].._NAME..[[.fontName = "font name" | foo = MW.fontName | foo = Bar.fontName

You can change the fontName property on the module which will affect MiniWindows subsequently returned by the New() function.
This property on MiniWindows and Bars shouldn't be changed by your scripting directly. Use the Font() function to change the loaded font.]], --']]
fontPadding = [[
Applies To: ALL
Prototype: MW.fontPadding = n

The number of pixels to offset the font placement from the top of the cell.]],
fontPitchAndFamily = [[
Applies To: MiniWindow, Bar
Prototype:  Bar.fontPitchAndFamily = number

This property can be used to specify the default value for the Font function, or be used to retrieve its last set value.
See MushClient's WindowFont documentation for more information]], --']]
fontSize = [[
Applies To: MiniWindow, Bar
Prototype:  Bar.fontSize = bool

This property can be used to specify the default value for the Font function, or be used to retrieve its last set value.
See MushClient's WindowFont documentation for more information]], --']]
fontStrikeout = [[
Applies To: MiniWindow, Bar
Prototype:  Bar.fontStrikeout = bool

This property can be used to specify the default value for the Font function, or be used to retrieve its last set value.
See MushClient's WindowFont documentation for more information]], --']]
fontUnderline = [[
Applies To: MiniWindow, Bar
Prototype:  Bar.fontUnderline = bool

This property can be used to specify the default value for the Font function, or be used to retrieve its last set value.
See MushClient's WindowFont documentation for more information]], --']]
rows = [[
Applies To: MiniWindow
Prototype: MW.rows = n

The number of rows. Setting this value directly bypasses validation in the Rows function.

n = number; Expected to be a positive, non-zero integer.]],
textStyle = [[
Applies To: Bar
Prototype: Bar.textStyle = n

The adornment on the caption text.

n = number; Expected values range from 0-5. See textStyles for more information]],
textStyles = [[
Applies To: All
Prototype: N/A

The table that enumerates textStyle values. Matting can be added to the other text styles for a little more clarity.

plain = 0;
matte = 1;
raised = 2;
sunken = 4;]],
windowFlags = [[
Applies To: Module, MiniWindow
Prototype: ]].._NAME..[[.windowFlags = n

Flags passed to the MushClient WindowCreate function. See MushClient documentation for more information.]],
windowHeight = [[
Applies To: Module, MiniWindow
Prototype: N/A

The MiniWindow.windowHeight is set dynamically to be .rows * .cellHeight + some padding. Setting this value manually is not recommended.]],
windowPosition = [[
Applies To: Module, MiniWindow
Prototype: ]].._NAME..[[.windowPosition = n

Setting this on the module will set the initial placement of all MiniWindows returned by the New() function. It is recommended to not set this value directly on MiniWindows, instead use the MW:WindowPosition() function.

n= number; Expected values range from 4-11. See windowPositions table for more information.]],
windowPositions = [[
Applies To: All
Prototype: N/A

Table enumerating windowPosition values. Placements are named after the cardinal points of the compass.

NE =  4;
N  =  5;
NW =  6;
E  =  7;
SE =  8;
S  =  9;
SW = 10;
W  = 11;]],
windowWidth = [[
Applies To: Module, MiniWindow
Prototype: MW.windowWidth = n

The Number of pixels the miniwindow is wide.

n = number; Expected to be a positive integer]],
AnchorRight = [[
Applies To: Bar
Prototype: Bar:AnchorRight(bool)

If True, then the 0 value is drawn on the right side of the gauge and captionPlacement, if set to innerLeft, is set to innerRight.]],
BadColour = [[
Applies To: Bar
Prototype: Bar:BadColour(colour)

Set the color for close to 0 values.

colour = string|number; colour can be the name of a color ("red"), an HTML formatted string ("#FF0000"), or a number (255)]],
BarStyle = [[
Applies To: Bar
Prototype: Bar:BarStyle(n)

A bitmask number representing the frame + fill desired for the bar.

n = number; 0 is text only, see barStyles for other values]],
Caption = [[
Applies To: Bar
Prototype: Bar:Caption(label)

Sets the text for the bar. If label is a number, commas are added for readability.

label = string|number; The desired text to print.]],
ColourText = [[
Applies To: Bar
Prototype: Bar:ColourText(val, threshold, good, bad [, neut])

Sets the textColour value to specified color.
Last three paramters can be specified as a name of a color ("red"), an HTML formatted string ("#FF0000"), or a number (255)

val = number; The value to be checked.
threshold = number; the value to check against.
good :: val > threshold (or val >=threshold if neut is not specified)
bad  :: val < threshold
neut :: val == threshold]],
GoodColour = [[
Applies To: Bar
Prototype: Bar:GoodColour(val)

The color for solid bars with value > threshold, or the non-zero side of gradients.]],
Padding = [[
Applies To: Bar
Prototype: Bar:Padding(n)

The number of pixels to add to the bottom of a gauge before drawing the next gauge in the column.

n = number; A number of pixels. Negative values will cause overlapping gauges.]],
TextColour = [[
Applies To: Bar
Prototype: Bar:TextColour(val)

The default color for text to be printed in if no other colour codes are specified in the caption value. If colour codes are specified, this function sets the value of the @~ identifier.

val = string|number; name of a color ("red"), an HTML formatted string ("#FF0000"), or a number (255)]],
TextStyle = [[
Applies To: Bar
Prototype: Bar:TextStyle(n)

Sets the adornments to printed text.

n = number; value from 0-5. see textStyles for more information.]],
Threshold = [[
Applies To: Bar
Prototype: Bar:Threshold(n)

Sets the value that goodColour transitions to badColour for bars with a solid barstyle, or sets where the midpoint of the gradient is for gradientFixed barstyles.

n = number; a value from 0 to 100.]],
Value = [[
Applies To: Bar
Prototype: Bar:Value(n)

Sets what percent of the gauge is filled. Values over 100 will be drawn as 100% with goodColour 15% brighter, if possible.

n = number; Percent to draw in the gauge. ]],
WatchCaption = [[
Applies To: Bar
Prototype: Bar:WatchCaption(varname)

Specifies the name of a variable that contains the text displayed for a given bar. Unlike the Caption function, a variable containing a number will not display the additional commas.

varname = string; the name of a variable. ("foo", "tbl.foo")]],
WatchValue = [[
Applies To: Bar
Prototype: Bar:WatchValue(varname)

Specifies the name of a variable that contains the percent of the gauge to draw.

varname = string; the name of a variable. ("percentHP", "tbl.nested[fighterhealth]")]],
anchorRight = [[
Applies To: Bar
Prototype: Bar.anchorRight = bool

Value that determines which side of the gauge has the 0 value. You can set this value directly safely to a boolean value. ]],
badColour = [[
Applies To: Bar
Prototype: Bar.badColour = n

The number value of the color drawn near 0 values for gradients, or when value is less than or equal to threshold for solid fills. The BadColour function sets this value to the number equivalent to a color.
Bar:BadColour("indigo") -- ok
Bar.badColour = 8519755 -- ok
Bar.badColour = "indigo" -- NOT OK!!!!

n = number; the number value for a colour.]],
barStyle = [[
Applies To: Bar
Prototype: Bar.barStyle = n

The bitmasked number determining how a bar is drawn. See barStyles for the enumeration of possible styles.

n= number;]],
caption = [[
Applies To: Bar
Prototype: Bar.caption = txt

The text to display for a bar's label. This value does not modify numbers when printing.
Bar:Caption(1000) --> prints 1,000
Bar.caption = 1000 --> prints 1000

Additional color settings can be specified with an "@x" delimiter where x is a single character. The 16 default ansi colors are taken from MushClient's settings and are set to c,m,y,k,r,g,b,w and their bold values C,M,Y,K... The colour that's in the textColour value can be referred to with "@~". Additional colours can be specified by adding a table to the customColourCodes value with a single character key, and the number value of the colour.
"@@" is needed to actually print an @ symbol in a caption.
i.e.:
MW.customColourCodes = {z = 255}
MW.Bars[2].customColourCodes = {z = 128}
MW.Bars[1].caption = "@zVery Red"
MW.Bars[2].caption = "@zNot so red"
MW.Bars[3].caption = "@zVery Red @~textColour Colour"

txt = string; the label.]], --']]
captionPlacement = [[
Applies To: Bar
Prototype: Bar.captionPlacement = n

The value of where the caption is placed. It is recommended to set this value with the CaptionPlacement() function instead of setting the value directly.

n = number; a value ranging from 0 to 5]],
cellHeight = [[
Applies To: Bar
Prototype: Bar.cellHeight = n

The number of pixels from the top of the bar to the bottom of the gauge. Note that the padding value is added to this to determine the top of the following gauge. This value is calculated dynamically when a font is loaded.
This should be a lesser used value since it is mostly handled internally.
n = number; ]],
cellPadding = [[
Applies To: Bar
Prototype: Bar.cellPadding = n

Used mostly internally, this value is added to the height of a font to make the gauge a little bigger. It is only used to calculate cellHeight when a font is loaded.

n = number; number of pixels]],
gaugeHeight = [[
Applies To: Bar
Prototype: Bar.gaugeHeight = n

Value that determines the top edge of the gauge. The internal draw routine uses cellHeight - gaugeHeight to determine coordinates for drawing so that gauges are naturally aligned to the bottom of the cell.

n = number; the number of pixels high a gauge should be]],
gaugeLeft = [[
Applies To: Bar
Prototype: Bar.gaugeLeft = n

The number of pixels the gauge appears from the edge of the cell. Recalculated automatically by the CaptionPlacement function.

n = number; number of pixels.]],
goodColour = [[
Applies To: Bar
Prototype: Bar.goodColour = n

The number value of the color drawn near 100% values for gradients, or when value is greater than threshold for solid fills. The GoodColour function sets this value to the number equivalent to a color.
Bar:GoodColour("indigo") -- ok
Bar.goodColour = 8519755 -- ok
Bar.goodColour = "indigo" -- NOT OK!

n = number; the number value for a colour.]],
padding = [[
Applies To: Bar
Prototype: Bar.padding = n

The number of pixels from the bottom of this gauge to the top of the next cell.

n = number; numberof pixels]],
textColour = [[
Applies To: Bar
Prototype: Bar.textColour = n

The number value of the colour for printed text. If there are no colour codes specified in the caption. See caption for more information.]],
textStyle = [[
Applies To: Bar
Prototype: Bar.textStyle = n

n = number; 0-5. see textStyles for details.]],
threshold = [[
Applies To: Bar
Prototype: Bar.threshold = n

The percentage value where goodColour changes to badColour for solid fills, or where the midpoint of the gradient is for gradientFixed fills.

n = number; ]],
value = [[
Applies To: Bar
Prototype: Bar.value = n

The percentage of the bar to draw. Values greater than 100% will draw as 100% with the colour 15% brighter, if possible. Values lower than 0 don't draw.

n = number; percent of gauge to draw]], --']]
id = [[
Applies To: Bar
Prototype:  Bar.id = n

It is recommended that you don't change this value. It is maintained internally. This value contains the index of this bar in the Bars table. It is useful for situations where you're dynamically adding and removing bars but kept a variable reference returned from the AddBar or InsertBar functions and may have lost track of which index this bar may be.]],
Bar = [[
Applies To: MiniWindow, Module
Prototype:  MW.Bar = {...}

The Bar table contains the default values for newly created bars. A newly created Miniwindow (returned from the ]].._NAME..[[:New() function) has a new table with the values from ]].._NAME..[[.Bar copied into it. This table has a metatable with its __index value referring to the miniwindow that contains it.]],
Bars = [[
Applies To: MiniWindow
Prototype: MW.Bars = {...}

The Bars table contains nested tables of duplicated Bar tables with customized values. The Update function gets the settings for what to draw by looping through the Bars table's subtables. ]], --']]
gaugeWidth = [[Applies To: Bar
Prototype:  Bar.gaugeWidth = n

The number of pixels the gauge extends within a bar. This property's default value is implemented in a metatable to dynamically return the appropriate width. Setting a value trumps the __index calculation, of course.

n = number; the number of pixels from the left edge ot the gauge to the right edge.]], --']]
["*Inheritance"] = [[Applies To: General Info
Prototype: N/A

This topic does not relate to one specific value or function. Its purpose is to document how the the metatables __index link.
Assuming:
  MW = InfoBox:New()
  MW:AddBar("Test")
  print (MW.Bars[1].foo)

  will search for a value foo in
  MW.Bars[1],
  MW.Bar,
  MW,
  then InfoBox
  ]],
  
["button"] =[[
Applies To: Bar
Protoype:   bar.button = {mouseUp = "funcname", ...}


This value expects a table with certain keys paired to the _names_ of functions you write. If there is a table present a Hotspot is added that is drawn over the entire region of the cell (not just the gauge).
Missing keys are fine. (i.e. {mouseUp="f"} is OK. {mouseUp="f", mouseDown="", cancelMouseDown=""... is not necessary.)

Expected keys are named:
mouseUp
mouseDown
cancelMouseDown
tooltipText
mouseOver
cancelMouseOver
cursor

Your functions should be structured as: 
   function myClickFunction (flags, strBarsIndex)
     local id = tonumber(strBarsIndex)  -- because tbl["1"] is a different key than tbl[1]
     ...
     MW.Bars[id].caption = "Clicked"
   end
]],

["cellTop"] =[[
Applies To: Bar
Protoype:   x = bar.cellTop

This value is set by the Update function. It is provided if you wish to add any custom coding and need to know where a bar was drawn. Setting this value produces no effect and will be overwritten the next time the Update function is called.
]],

["cellLeft"] =[[
Applies To: Bar
Protoype:   x = bar.cellLeft

This value is set by the Update function. It is provided if you wish to add any custom coding and need to know where a bar was drawn. Setting this value produces no effect and will be overwritten the next time the Update function is called.
]],

["Fade"] =[[
Applies To: Bar
Protoype:   bar:Fade(bool)

Calling this function sets the fade value and recalculates the shades table to which the fade effect refers. It's not required to call this function, since the draw routine will recalculate the shades table if necessary. There is an interesting bug that can be exploited by calling this function however.

Try the following:
 for x = 100,30,-10 do
   local foo = MW:AddBar("", x, "green", "red", false, MW.barStyles.glass + MW.barStyles.gradientScale)
   foo:Fade(true)
   foo.badColour = colour_names.dodgerblue
 end

It's a bug, but it looks cool. Also this is why using the functions to set values is recommended!
]],

["cellWidth"] =[[
Applies To: Bar 
Protoype:   x = bar.cellWidth

The value for this property is generated by the Miniwindow's __index metamethod. The function in the metamethod returns (windowWidth / columns) for you. You should never have a reason to set your own value, and doing so will probably break things in uncool ways.
]], --']]

["*AddYourOwn"] =[[
Applies To: ALL
Protoype:   N/A

A Bar is a table, after all, and you might find it useful to attach your own data, or a reference, to a bar. This is mostly safe to do as the module uses the pairs() and ipairs() functions in limited places. 
pairs() is used to copy InfoBox.Bar to MW.Bar
ipairs() is used on the MW.Bar table to find the functions for the default parameters for AddBar.
ipairs() is used on the MW.Bars table to draw the bars.

Other than avoiding numerical indices in the 2 Bar(s) tables, it should be safe to add to a table as you see fit.
]],

--[=[ 
[""] =[[
Applies To: 
Protoype:   


]],

--]=]-- ']]
}

function Doc (self, topic)
    if topic and _Doc[topic] then
        print()
        print (_Doc[topic],"\n")
    else
        local p = require "pairsByKeys"
        print ("\nAdditional help can be found on the following functions and values\n")
        local t , i={}, 0
        for k,v in p(_Doc) do  -- t[#t+1] = k end
            local s= "%-20s"
            i = i+1
            Tell(s:format(k))
            if i == 4 then
                print()
                i=0
        end end
        print("\n\nSyntax: ".. GetInfo(36) .. _NAME .. ':Doc("topic")\n')
end end
