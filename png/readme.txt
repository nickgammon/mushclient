LICENSE
-------

See the file LICENSE for the libpng license details.

Note that the libpng source is unchanged except as noted below.


BUILDING A NEW VERSION
----------------------

PNG built as follows:

1. Download recent version from download.sourceforge.net

    In this case: http://download.sourceforge.net/libpng/libpng-1.6.37.tar.gz


2. Uncompress the archive with 7-Zip (if necessary), obtainable from:

	http://www.7-zip.org/


3. If backing up the old version, leave behind the file pnglibconf.h which is the configuration file.

4. Copy the following files to the mushclient source "png" directory:

	png.c
	png.h
	pngconf.h
	pngdebug.h
	pngerror.c
	pngget.c
	pnginfo.h
	pngmem.c
	pngpread.c
	pngpriv.h
	pngread.c
	pngrio.c
	pngrtran.c
	pngrutil.c
	pngset.c
	pngstruct.h
	pngtrans.c
	pngwio.c
	pngwrite.c
	pngwtran.c
	pngwutil.c

5.  Copy scripts/pnglibconf.h.prebuilt to pnglibconf.h


6. Edit: pngstruct.h and change

#include "zlib.h"

to

#include "../zlib/zlib.h"


Also add to the very start of png.h:

// pragmas added by Nick Gammon
#pragma warning( disable : 4115)  // named type definition in parentheses
#pragma warning( disable : 4701)  // local variable 'x' may be used without having been initialized
