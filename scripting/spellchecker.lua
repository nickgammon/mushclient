--[[
  Spell checker for MUSHclient, written by Nick Gammon.
  Written:  9th October 2006
  Updated: 11th October 2006
  Updated:  6th March 2007 to make progress bar optional
  Updated: 13th April 2007 to added IGNORE_MIXED_CASE, IGNORE_IMBEDDED_NUMBERS
  Updated: 15th February 2009 to convert to using SQLite database instead of Lua table
  Updated: 21st February 2009 to fix problem where words with 2 metaphones were only stored once.
--]]

local SHOW_PROGRESS_BAR = true -- show progress bar? true or false

local METAPHONE_LENGTH = 4   -- how many characters of metaphone to get back
local EDIT_DISTANCE = 4      -- how close a word must be to appear in the list of suggestions
local CASE_SENSITIVE = false -- compare case? true or false
local IGNORE_CAPITALIZED = false -- ignore words starting with a capital? true or false
local IGNORE_MIXED_CASE = false  -- ignore words in MixedCase (like that one)? true or false
local IGNORE_IMBEDDED_NUMBERS = false -- ignore words with numbers in them? true or false

-- this is the pattern we use to find "words" in the text to be spell-checked
local pattern = "%a+'?[%a%d]+" -- regexp to give us a word with a possible single imbedded quote

-- path to the spell check dictionaries
local directory = utils.info ().app_directory .. "spell\\"
-- file name of the user dictionary, in the above path
local userdict = "userdict.txt"

-- stuff below used internally
local make_upper  -- this becomes the upper-case conversion function, see below
local db          -- SQLite3 dictionary database
local cancelmessage = "spell check cancelled"
local previousword  --> not used right now
local change, ignore  -- tables of change-all, ignore-all words

-- dictionaries - add new entries along similar lines to add more dictionary files
local files = {

-- lower-case words

  "english-words.10",
  "english-words.20",
  "english-words.35",
  "english-words.40",

-- upper case words

  "english-upper.10",
  "english-upper.35",
  "english-upper.40",

-- American words

  "american-words.10",
  "american-words.20",

-- contractions (eg. aren't, doesn't)

  "english-contractions.10",
  "english-contractions.35",
  
-- user dictionary
  userdict,
  }
  
 -- trim leading and trailing spaces from a string
local function trim (s)
  return (string.gsub (s, "^%s*(.-)%s*$", "%1"))
end -- trim

-- insert a word into our metaphone table - called by reading dictionaries
-- and also by adding a word during the spellcheck
local function insert_word (word, user)
  
  if word == "" then
    return
  end -- empty word
  
              
  -- get both metaphones
  local m1, m2 = utils.metaphone (word, METAPHONE_LENGTH)
  local fixed_word = string.gsub (word, "'", "''")  -- convert ' to ''
  
  assert (db:execute (string.format ("INSERT INTO words VALUES (NULL, '%s', '%s', %i)",
            fixed_word, m1, user)));
    
  -- do 2nd metaphone, if any
   if m2 then
    assert (db:execute (string.format ("INSERT INTO words VALUES (NULL, '%s', '%s', %i)",
              fixed_word, m2, user)));
   end -- having alternative      
  
end -- insert_word


-- sort function for sorting the suggestions into edit-distance order
local function suggestions_compare (word)
   return function (a, b)
     local diff = utils.edit_distance (make_upper (a), word) - 
                  utils.edit_distance (make_upper (b), word) 
     if diff == 0 then
       return make_upper (a) < make_upper (b)
     else
       return diff < 0
     end -- differences the same?
  end -- compareit
end -- function suggestions_compare

-- check for one word, called by spellcheck (invokes suggestion dialog)
local function checkword_and_suggest (word)

  if IGNORE_CAPITALIZED then
    -- ignore words starting in caps
    if string.find (word, "^[A-Z]") then
      return word, "ignore"
    end -- this round, ignore this word
  end -- if IGNORE_CAPITALIZED

  if IGNORE_MIXED_CASE  then
    -- ignore words in mixed case
    if string.find (word, "[A-Z]") and
       string.find (word, "[a-z]") then
      return word, "ignore"
    end -- this round, ignore this word
  end -- if IGNORE_MIXED_CASE

  if IGNORE_IMBEDDED_NUMBERS  then
    -- ignore words with numbers in them
    if string.find (word, "%d") then
      return word, "ignore"
    end -- this round, ignore this word
  end -- if IGNORE_IMBEDDED_NUMBERS

  uc_word = make_upper (word)  -- convert to upper-case if wanted

  -- if we already did "ignore all" on this particular word, ignore it again
  if ignore [word] then
    return word, "ignore"
  end -- this round, ignore this word
  
  -- if we said change A to B, change it again
  if change [word] then
    return change [word], "change"
  end -- change to this word
  
  -- table of suggestions, based on the metaphone
  local keyed_suggestions = {}
  
  -- get both metaphones
  local m1, m2 = utils.metaphone (word, METAPHONE_LENGTH)
  
  local function lookup_metaphone (m)
    local found = false
    for row in db:rows(string.format ("SELECT name FROM words WHERE metaphone = '%s'", m)) do
      local word = row [1]
      if make_upper (word) == uc_word then
        found = true -- found exact match
        break
      end  -- found
      if utils.edit_distance (make_upper (word), uc_word) < EDIT_DISTANCE then
        keyed_suggestions [word] = true
      end -- close enough
    end
    return found
  end -- lookup_metaphone
  
  -- look up first metaphone
  if lookup_metaphone (m1) then
    return word, "ok"
  end -- word found

  -- try 2nd metaphone
  if m2 then
    if lookup_metaphone (m2) then
      return word, "ok"
    end -- word found
  end -- have alternate metaphone

  -- pull into indexed table
  local suggestions = {}
  for k in pairs (keyed_suggestions) do
    table.insert (suggestions, k)
  end -- for 
     
  table.sort (suggestions, suggestions_compare (uc_word))
  
  -- not found? do spell check dialog
  local action, replacement = utils.spellcheckdialog (word, suggestions)
  
  -- they cancelled?
  if not action then
    error (cancelmessage)  --> forces us out of gsub loop
  end -- cancelled
  
  -- ignore this only - just return
  if action == "ignore" then
    return word, "ignore" -- use current word
  end -- ignore word

  -- ignore all of this word? add to list
  if action == "ignoreall" then
    ignore [word] = true
    return word, "ignore" -- use current word
  end -- ignore word

  -- add to user dictionary? 
  -- add to metaphone table, and rewrite dictionary
  if action == "add" then
    insert_word (word, 1)
    return word, "ok"
  end -- adding
  
  -- change word once? return replacement
  if action == "change" then
     return checkword_and_suggest (replacement)  -- however, re-check it
  end -- changing
  
  -- change all occurrences? add to table, return replacement
  if action == "changeall" then
    local newword, newaction = checkword_and_suggest (replacement) -- re-check it
    if newaction == "ok" then
       change [word] = newword
    end -- if approved
    return newword  -- return the new word
  end -- changing
  
  error "unexpected result from dialog"
end -- checkword_and_suggest

-- exported function to do the spellcheck
function spellcheck (line)
  change = {}  -- words to change
  ignore = {}  -- words to ignore
  
  -- we raise an error if they cancel the spell check dialog
  ok, result = xpcall (function () 
                        return string.gsub (line, pattern, checkword_and_suggest) 
                      end, debug.traceback)
 
  if ok then
    return result
  end -- not cancelled spell check

  -- whoops! syntax error?
  if not string.find (result, cancelmessage, 1, true) then
    error (result)
  end -- some syntax error
  
  return nil  --> shows they cancelled
end -- spellchecker

local notfound  -- table of not-found words, for spellcheck_string

-- check for one word, called by spellcheck_string
local function checkword (word)
  uc_word = make_upper (word)  -- convert to upper-case if wanted

  -- get first metaphone 
  local m = utils.metaphone (word, METAPHONE_LENGTH)
  
  local found = false
  for row in db:rows(string.format ("SELECT name FROM words WHERE metaphone = '%s'", m)) do
    local word = row [1]
    if make_upper (word) == uc_word then
      found = true -- found exact match
      break
    end  -- found
  end
  
  if found then return end  -- do nothing if word found

  -- otherwise insert our word   
  table.insert (notfound, word) 
end -- function checkword

-- exported function to spellcheck a string
function spellcheck_string (text)
  notfound = {}
  string.gsub (text, pattern, checkword)
  return notfound
end -- spellcheck_string

-- exported function to add a word to the user dictionary
function spellcheck_add_word (word, action, replacement)
  assert (action == "i", "Can only use action 'i' in user dictionary")  -- only "i" supported right now
  insert_word (word, 1)
end -- spellcheck_string

-- read one of the dictionaries
local function read_dict (dlg, name)
 if SHOW_PROGRESS_BAR then
   dlg:step ()
   dlg:status (directory .. name)
   if dlg:checkcancel () then
     error "Dictionary loading cancelled"
   end -- if cancelled
 end -- if SHOW_PROGRESS_BAR

  for line in io.lines (directory .. name) do
    insert_word (line, 0)
  end 
end -- read_dict
  
local function init ()

  -- make a suitable function depending on whether they want case-sensitive or not
  if CASE_SENSITIVE then
    make_upper = function (s) return s end -- return original
  else
    make_upper = function (s) return s:upper () end  -- make upper case
  end -- case-sensitivity test
  
  -- if no user dictionary, create it
  local f = io.open (directory .. userdict, "r")
  if not f then
    f = io.output (directory .. userdict)
    f:close ()
  else
    f:close ()
  end -- checking for user dictionary
 
  -- open database on disk
  db = assert (sqlite3.open( directory .. "spell.sqlite"))
  local words_table = false
  local count = 0
  
  -- if database just created, there won't be a words table
  for row in db:nrows("SELECT * FROM sqlite_master WHERE type = 'table' AND name = 'words'") do 
    if string.match (row.sql, "word_id") then  -- better be newer version
      words_table = true
    end -- if
  end
  
  -- enable WAL (Write-Ahead Logging)
  assert (db:execute "PRAGMA journal_mode=WAL;")
  
  -- if no words table, make one
  if not words_table then
      -- create a table to hold the words
    assert (db:execute[[
      DROP TABLE IF EXISTS words;
      CREATE TABLE words(
        word_id INTEGER NOT NULL PRIMARY KEY autoincrement,
        name  VARCHAR(10) NOT NULL,
        metaphone VARCHAR(10) NOT NULL,
        user INT(1)
      );
      CREATE INDEX metaphone_index ON words (metaphone);
      CREATE INDEX name_index ON words (name);
    ]])
  end -- if 
    
  -- check if table empty
  for row in db:rows('SELECT COUNT(*) FROM words') do 
    count = row [1] 
  end

  -- if empty, populate it  
  if count == 0 then
    
    local dlg
  
    if SHOW_PROGRESS_BAR then
      dlg = progress.new ("Loading dictionaries into SQLite database ...")
    
      dlg:range (0, #files)
      dlg:setstep (1)
    end -- if SHOW_PROGRESS_BAR
     
    assert (db:execute "BEGIN TRANSACTION");
    
    for k, v in ipairs (files) do
      ok, result = pcall (function () 
                            read_dict (dlg, v) 
                          end)
      if not ok then 
        if SHOW_PROGRESS_BAR then
          dlg:close ()
        end -- if SHOW_PROGRESS_BAR
        error (result)
      end -- not ok
    end -- reading each file
    
    if SHOW_PROGRESS_BAR then
      dlg:close ()
    end -- if SHOW_PROGRESS_BAR
    
    assert (db:execute "COMMIT");
    
  end -- if nothing in database
  
  
end -- init

-- when script is loaded, do initialization stuff

init ()

