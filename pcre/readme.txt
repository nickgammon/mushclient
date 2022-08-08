LICENSE
-------

See the file LICENSE for the PCRE license details.

Note that the PCRE source is unchanged except as noted below.


BUILDING A NEW VERSION
----------------------

PCRE built as follows:

1. Download: https://sourceforge.net/projects/pcre/files/pcre/8.45/pcre-8.45.zip/download

2. Unzip the file pcre-8.45.zip


3. Copy the following files to the mushclient source "pcre" directory:


pcre.h.generic
pcre_chartables.c.dist

pcre_compile.c
pcre_config.c
pcre_dfa_exec.c
pcre_exec.c
pcre_fullinfo.c
pcre_get.c
pcre_globals.c
pcre_internal.h
pcre_maketables.c
pcre_newline.c
pcre_ord2utf8.c
pcre_refcount.c
pcre_study.c
pcre_tables.c
pcre_ucd.c
pcre_valid_utf8.c
pcre_version.c
pcre_xclass.c
ucp.h


(Note that config.h is part of the git repository)

4. Rename pcre_chartables.c.dist as pcre_chartables.c
   Rename pcre.h.generic as pcre.h

