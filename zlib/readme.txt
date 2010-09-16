zlib built as follows:

1. Download recent version from http://www.zlib.net/

2. Unzip and untar the file

3. Copy the following files to the mushclient source "zlib" directory:

adler32.c
compress.c
crc32.c
crc32.h
deflate.c
deflate.h
gzclose.c
gzguts.h
gzlib.c
gzread.c
gzwrite.c
inffast.c
inffast.h
inffixed.h
inflate.c
inflate.h
inftrees.c
inftrees.h
minigzip.c
trees.c
trees.h
uncompr.c
zconf.h
zlib.h
zutil.c
zutil.h


3. Edit: zconf.h and after the lines:

#ifndef ZCONF_H
#define ZCONF_H

add:

#undef ZLIB_DLL      // NJG


4. Edit: zlib.h and at the start add the lines:

// pragmas added by Nick Gammon
#pragma warning( disable : 4131)  // uses old-style declarator
#pragma warning( disable : 4127)  // conditional expression is constant
#pragma warning( disable : 4244)  // conversion from 'int' to 'unsigned short', possible loss of data
