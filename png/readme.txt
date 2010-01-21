PNG built as follows:

1. Download recent version from http://sourceforge.net/projects/libpng/files/

2. Unzip and untar the file

3. Copy the following files to the mushclient source "png" directory:

png.c
png.h
pngconf.h
pngerror.c
pnggccrd.c
pngget.c
pngmem.c
pngpread.c
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

