LuaCOM built as follows:

1. Download recent version from http://github.com/davidm/luacom/


2. Edit: LuaCompat.h and add the following lines to the start of it:

// pragmas added by Nick Gammon
#pragma warning ( disable : 4100)  // unreferenced formal parameter
#pragma warning ( disable : 4189)  // warning C4189: 'iVersion' : local variable is initialized but not referenced
#pragma warning ( disable : 4244)  // conversion from 'int' to 'unsigned short', possible loss of data
#pragma warning ( disable : 4310)  // cast truncates constant value
#pragma warning ( disable : 4245)  // conversion from 'long' to 'unsigned long', signed/unsigned mismatch
#pragma warning ( disable : 4127)  // conditional expression is constant
#pragma warning ( disable : 4701)  // local variable 'xxx' may be used without having been initialized
#pragma warning ( disable : 4702)  // unreachable code
#pragma warning ( disable : 4710)  // function 'x' not inlined
