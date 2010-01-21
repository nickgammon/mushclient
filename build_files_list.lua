-- builds a file containing every source file in it

assert (package.loadlib ("lua_utils.dll", "luaopen_utils")) ()

function scan_dir (path, f)

  -- find all files in that directory
  local t = assert (utils.readdir (path .. "\\*"))

  for k, v in pairs (t) do
   if not v.hidden and 
      not v.system and
      k:sub (1, 1) ~= "." then
      
      -- recurse to process file or subdirectory
      if v.directory then
        scan_dir (path .. "\\" .. k, f)
      else
        f (path .. "\\" .. k, v)
      end -- if 
     
   end -- if
  
  end -- for

end -- scan_dir 

files = {}

-- this function is called for every found file
function load_file (name, stats)
 
  if string.match (name:lower (), "%.cpp$") or
     string.match (name:lower (), "%.c$")then
       table.insert (files,  name)
  end -- if
  
end -- load_file

scan_dir ("C:\\source\\mushclient", load_file)


table.sort (files)

fo = assert (io.open ("all_files_list.txt", "w"))
for _, name in ipairs (files) do
  fo:write (name .. "\n")
end -- for

fo:close ()

