-- Convert a number to words
-- Author: Nick Gammon
-- Date: 18th March 2010

-- See: http://www.gammon.com.au/forum/?id=10155

-- Examples of use:
--    words  = convert_numbers_to_words ("94921277802687490518")
--    number = convert_words_to_numbers ("one hundred eight thousand three hundred nine")

-- Both functions return nil and an error message so you can check for failure,
-- or assert, eg. words = assert (convert_numbers_to_words ("2687490518"))

-- Units, must be in inverse order!
-- The trailing space is required as the space between words

local inverse_units = {
    "vigintillion ",     -- 10^63
    "novemdecillion ",   -- 10^60
    "octodecillion ",    -- 10^57
    "septendecillion ",  -- 10^54
    "sexdecillion ",     -- 10^51
    "quindecillion ",    -- 10^48
    "quattuordecillion ",-- 10^45
    "tredecillion ",     -- 10^42
    "duodecillion ",    -- 10^39
    "undecillion ",     -- 10^36
    "decillion ",       -- 10^33
    "nonillion ",       -- 10^30
    "octillion ",       -- 10^27
    "septillion ",      -- 10^24
    "sextillion ",      -- 10^21
    "quintillion ",     -- 10^18
    "quadrillion ",     -- 10^15
    "trillion ",        -- 10^12
    "billion ",         -- 10^9
    "million ",         -- 10^6
    "thousand ",        -- 10^3
  } -- inverse_units
  
local inverse_numbers = {
    "one ",
    "two ",
    "three ",
    "four ",
    "five ",
    "six ",
    "seven ",
    "eight ",
    "nine ",
    "ten ",
    "eleven ",
    "twelve ",
    "thirteen ",
    "fourteen ",
    "fifteen ",
    "sixteen ",
    "seventeen ",
    "eighteen ",
    "nineteen ",
    "twenty ",
    [30] = "thirty ",
    [40] = "forty ",
    [50] = "fifty ",
    [60] = "sixty ",
    [70] = "seventy ",
    [80] = "eighty ",
    [90] = "ninety ",
 }  -- inverse_numbers
 
local function convert_up_to_999 (n)

  if n <= 0 then
    return ""
  end -- if zero
  
  local hundreds = math.floor (n / 100)
  local tens = math.floor (n % 100)
  local result = ""

  -- if over 99 we need to say x hundred  
  if hundreds > 0 then
  
    result = inverse_numbers [hundreds] .. "hundred "
    if tens == 0 then
      return result
    end -- if only a digit in the hundreds column
  
  -- to have "and" between things like "hundred and ten"
  -- uncomment the next line
  --  result = result .. "and "

  end -- if
  
  -- up to twenty it is then just five hundred (and) fifteen
  if tens <= 20 then
    return result .. inverse_numbers [tens] 
  end -- if

  -- otherwise we need: thirty (something)
  result = result .. inverse_numbers [math.floor (tens / 10) * 10] 
  
  -- get final digit (eg. thirty four)
  local digits = math.floor (n % 10)

  -- to put a hyphen between things like "forty-two" 
  -- uncomment the WITH HYPHEN line and 
  -- comment out the NO HYPHEN line

  if digits > 0 then
    result = result ..  inverse_numbers [digits]  -- NO HYPHEN
--    result = string.sub (result, 1, -2) .. "-" ..  inverse_numbers [digits]  -- WITH HYPHEN
  end -- if 

  return result
  
end -- convert_up_to_999

-- convert a number to words
-- See: http://www.gammon.com.au/forum/?id=10155

function convert_numbers_to_words (n)
  local s = tostring (n)
  
  -- preliminary sanity checks
  local c = string.match (s, "%D")
  if c then
    return nil, "Non-numeric digit '" .. c .. "' in number"
  end -- if

  if #s == 0 then
    return nil, "No number supplied"
  elseif #s > 66 then
    return nil, "Number too big to convert to words"
  end -- if
  
  -- make multiple of 3
  while #s % 3 > 0 do
    s = "0" .. s
  end -- while
      
  local result = ""
  local start = #inverse_units - (#s / 3) + 2
  
  for i = start, #inverse_units do
    local group = tonumber (string.sub (s, 1, 3))
    if group > 0 then
      result = result .. convert_up_to_999 (group) .. inverse_units [i]
    end -- if not zero
    s = string.sub (s, 4)    
  end -- for
  
  result = result .. convert_up_to_999 (tonumber (s)) 

  if result == "" then
    result = "zero"
  end -- if
  
  return (string.gsub (result, " +$", ""))  -- trim trailing spaces

end -- convert_numbers_to_words

-- Convert words to a number
-- Author: Nick Gammon
-- Date: 18th March 2010

-- Does NOT handle decimal places (eg. four point six)

local numbers = {
         zero       = bc.number (0),
         one        = bc.number (1),
         two        = bc.number (2),
         three      = bc.number (3),
         four       = bc.number (4),
         five       = bc.number (5),
         six        = bc.number (6),
         seven      = bc.number (7),
         eight      = bc.number (8),
         nine       = bc.number (9),
         ten        = bc.number (10),
         eleven     = bc.number (11),
         twelve     = bc.number (12),
         thirteen   = bc.number (13),
         fourteen   = bc.number (14),
         fifteen    = bc.number (15),
         sixteen    = bc.number (16),
         seventeen  = bc.number (17),
         eighteen   = bc.number (18),
         nineteen   = bc.number (19),
         twenty     = bc.number (20),
         thirty     = bc.number (30),
         forty      = bc.number (40),
         fifty      = bc.number (50),
         sixty      = bc.number (60),
         seventy    = bc.number (70),
         eighty     = bc.number (80),
         ninety     = bc.number (90),
} -- numbers 

local units = {
          hundred             = bc.number ("100"),
          thousand            = bc.number ("1" .. string.rep ("0",  3)),
          million             = bc.number ("1" .. string.rep ("0",  6)),
          billion             = bc.number ("1" .. string.rep ("0",  9)),
          trillion            = bc.number ("1" .. string.rep ("0", 12)),
          quadrillion         = bc.number ("1" .. string.rep ("0", 15)),
          quintillion         = bc.number ("1" .. string.rep ("0", 18)),
          sextillion          = bc.number ("1" .. string.rep ("0", 21)),
          septillion          = bc.number ("1" .. string.rep ("0", 24)),
          octillion           = bc.number ("1" .. string.rep ("0", 27)),
          nonillion           = bc.number ("1" .. string.rep ("0", 30)),
          decillion           = bc.number ("1" .. string.rep ("0", 33)),
          undecillion         = bc.number ("1" .. string.rep ("0", 36)),
          duodecillion        = bc.number ("1" .. string.rep ("0", 39)),
          tredecillion        = bc.number ("1" .. string.rep ("0", 42)),     
          quattuordecillion   = bc.number ("1" .. string.rep ("0", 45)),
          quindecillion       = bc.number ("1" .. string.rep ("0", 48)),    
          sexdecillion        = bc.number ("1" .. string.rep ("0", 51)),     
          septendecillion     = bc.number ("1" .. string.rep ("0", 54)),  
          octodecillion       = bc.number ("1" .. string.rep ("0", 57)),    
          novemdecillion      = bc.number ("1" .. string.rep ("0", 60)),   
          vigintillion        = bc.number ("1" .. string.rep ("0", 63)),     
  } -- units
  
-- convert a number in words to a numeric form
-- See: http://www.gammon.com.au/forum/?id=10155
-- Thanks to David Haley

function convert_words_to_numbers (s)

  local stack = {}
  local previous_type
 
  for word in string.gmatch (s:lower (), "[%a%d]+") do
    if word ~= "and" then  -- skip "and" (like "hundred and fifty two")
      local top = #stack
      
      -- If the current word is a number (English or numeric), 
      -- and the previous word was also a number, pop the previous number 
      -- from the stack and push the addition of the two numbers. 
      -- Otherwise, push the new number.

      local number = tonumber (word)  -- try for numeric (eg. 22 thousand)

      if number then
        number = bc.number (number)   -- turn into "big number"
      else
        number = numbers [word]
      end -- if a number-word "like: twenty"

      if number then
        if previous_type == "number" then   -- eg. forty three
          local previous_number = table.remove (stack, top)  -- get the forty
          number = number + previous_number  -- add three
        end -- if 
        table.insert (stack, number)   
        previous_type = "number"
      else
      
        -- If the current word is a unit, multiply the number on the top of the stack by the unit's magnitude. 
        local unit = units [word]
        if not unit then
          return nil, "Unexpected word: " .. word
        end -- not unit
        previous_type = "unit"
        
        -- It is an error to get a unit before a number.
        
        if top == 0 then
          return nil, "Cannot have unit before a number: " .. word
        end -- starts of with something like "thousand"

        -- pop until we get something larger on the stack
        local interim_result = bc.number (0)
        while top > 0 and stack [top] < unit do
          interim_result = interim_result + table.remove (stack, top)
          top = #stack
        end -- while
        table.insert (stack, interim_result * unit)
               
      end -- if number or not
    end -- if 'and'

  end -- for each word
  
  if #stack == 0 then
    return nil, "No number found"
  end -- nothing
  
  -- When the input has been parsed, sum all numbers on the stack.
  
  local result = bc.number (0)
  for _, item in ipairs (stack) do
    result = result + item
  end -- for
  
  return result
end -- function convert_words_to_numbers
