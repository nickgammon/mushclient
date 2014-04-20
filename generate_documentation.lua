-- Documenation SQL fixer-upper

-- Author: Nick Gammon
-- Date: 12th June 2011

-- Converts documentation from MySQL to SQLite3 format

-- See: http://www.gammon.com.au/forum/bbshowpost.php?id=9241

local fixups = {
 ["\\a"] = "\a",      -- Bell (alert)         
 ["\\b"] = "\b",      --  Backspace            
 ["\\f"] = "\f",      --  Formfeed             
 ["\\n"] = "\n",      --  Newline      
 ["\\r"] = "",        --  Carriage return      
 ["\\t"] = "\t",      --  Horizontal tab       
 ["\\v"] = "\v",      --  Vertical tab         
 ["\\'"] = "''",      --  Single quotation mark
 ["\\\""] = "\"",     --  Double quotation mark
 ["\\\\"] = "\\",     --  Backslash            
 ["\\?"] = "?",       --  Literal question mark
 }
 
 
function FixupEscapeSequences (s)

 -- look for backslash-something 
 s = string.gsub (s, "\\.", fixups)

 -- handle newlines
-- s = string.gsub (s, "\\n", "' || x'0A' || '")
 
 --  \xhh ASCII character in hexadecimal notation
 s = string.gsub (s, "\\[xX](%x%x)", function (s) return string.char (tonumber (s, 16)) end)
 
 --  &#x5C; character fixup
 s = string.gsub (s, "&#[xX](%x%x);", function (s) return string.char (tonumber (s, 16)) end)

 
 -- fix up newlines
-- return string.gsub (s, "\n", "'\n")
 
return s
end -- function FixupEscapeSequences
 
fi = assert (io.open ("\\\\10.0.0.2\\general\\documentation.sql", "r"))

fo = assert (io.open ("documentation_fixed.sql", "w"))

fo:write ("-- MUSHclient documentation\n\n")
fo:write (os.date ("-- Written: %A, %d %B %Y at %H:%M:%S\n\n"))

fo:write ("BEGIN TRANSACTION;\n\n")

local line = fi:read () -- priming read
local count
local add_index
while line do

  line = FixupEscapeSequences (line)
  
  -- fix UNIQUE KEY problem
  line = string.gsub (line, "  UNIQUE KEY `[%w%d_]-`", "  UNIQUE ")
  -- fix quotes inside strings
  --line = string.gsub (line, "\\'", "''")
  
  -- get rid of enum
  line = string.gsub (line, "  `type_of_object` enum%('Method','Property'%) NOT NULL DEFAULT 'Method',",
    "  `type_of_object` varchar(10) NOT NULL DEFAULT 'Method',")

   -- get rid of comma here   
  line = string.gsub (line, "  UNIQUE  %(`doc_name`,`xref_name`%),",
                            "  UNIQUE  (`doc_name`,`xref_name`)")

  -- get rid of keys
  line = string.gsub (line, "  KEY `doc_name_2` %(`doc_name`%),", "")
  line, count = string.gsub (line, "  KEY `xref_name` %(`xref_name`%)", "")
  
  if count > 0 then
    add_index = true
  end -- if
     
  fo:write (line, "\n")

  -- add keys back
  if add_index and string.match (line, "^%);") then
    fo:write ([[
    
  CREATE INDEX doc_name_2 ON general_doc_xref (doc_name);
  CREATE INDEX xref_name ON general_doc_xref (xref_name);
]])
  
    add_index = false
  end -- if
  
  line = fi:read () -- read next
end -- while

-- create full-text search tables

fo:write [[
DROP TABLE IF EXISTS commands_lookup;
DROP TABLE IF EXISTS dialogs_lookup;
DROP TABLE IF EXISTS functions_lookup;
DROP TABLE IF EXISTS general_doc_lookup;
DROP TABLE IF EXISTS errors_lookup;
DROP TABLE IF EXISTS lua_functions_lookup;

/*

-- do this at the client end to save download space

CREATE VIRTUAL TABLE commands_lookup USING FTS4(command_name, short_description, description);
INSERT INTO commands_lookup (command_name, short_description, description) SELECT command_name, short_description, description FROM commands;

CREATE VIRTUAL TABLE dialogs_lookup USING FTS4(dialog_name, title, description);
INSERT INTO dialogs_lookup (dialog_name, title, description) SELECT dialog_name, title, description FROM dialogs;

CREATE VIRTUAL TABLE functions_lookup USING FTS4(name, summary, description, lua_example, lua_notes);
INSERT INTO functions_lookup (name, summary, description, lua_example, lua_notes) SELECT name, summary, description, lua_example, lua_notes FROM functions;

CREATE VIRTUAL TABLE general_doc_lookup USING FTS4(doc_name, title, description);
INSERT INTO general_doc_lookup (doc_name, title, description) SELECT doc_name, title, description FROM general_doc;

CREATE VIRTUAL TABLE errors_lookup USING FTS4(error_name, error_code, meaning);
INSERT INTO errors_lookup (error_name, error_code, meaning) SELECT error_name, error_code, meaning FROM errors;

CREATE VIRTUAL TABLE lua_functions_lookup USING FTS4(name, summary, description);
INSERT INTO lua_functions_lookup (name, summary, description) SELECT name, summary, description FROM lua_functions;
*/

DROP TABLE IF EXISTS `tblUnixControl`;
]]

fo:write ("COMMIT;\n\n")

fo:close ()

fi:close ()

print "done"
