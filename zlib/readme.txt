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
gzio.c
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
