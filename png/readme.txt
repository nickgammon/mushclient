PNG built as follows:

1. Download recent version from http://sourceforge.net/projects/libpng/files/

    In this case: lpng143.7z

2. Uncompress the archive with 7-Zip, obtainable from:

	http://www.7-zip.org/


3. Copy the following files to the mushclient source "png" directory:

png.c
png.h
pngconf.h
pngerror.c
pngget.c
pngmem.c
pngpread.c
pngpriv.h
pngread.c
pngrio.c
pngrtran.c
pngrutil.c
pngset.c
pngtrans.c
pngwio.c
pngwrite.c
pngwtran.c
pngwutil.c


3. Edit: png.h and change

#include "zlib.h"

to

#include "../zlib/zlib.h"


Also add to the very start of png.h:

// pragmas added by Nick Gammon
#pragma warning( disable : 4115)  // named type definition in parentheses
