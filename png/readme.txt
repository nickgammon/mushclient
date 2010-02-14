PNG built as follows:

1. Download recent version from http://sourceforge.net/projects/libpng/files/

    In this case: libpng-1.4.0.tar.gz

2. Unzip and untar the file, eg.

   tar xzf libpng-1.4.0.tar.gz

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

