-- com.lua

-- Glue module to help load luacom

assert (package.loadlib ("luacom.dll", "luacom_open")) ()

return luacom
