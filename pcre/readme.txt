PCRE built as follows:

1. Download: http://sourceforge.net/projects/pcre/files/pcre/8.00/pcre-8.00.tar.bz2/download

2. Unzip and untar the file (eg.):

bunzip2 pcre-8.00.tar.bz2
tar xvf pcre-8.00.tar


2. Copy the following files to the mushclient source "pcre" directory:

chartables.c
pcre.h
pcre_chartables.c
pcre_compile.c
pcre_config.c
pcre_dfa_exec.c
pcre_exec.c
pcre_fullinfo.c
pcre_get.c
pcre_globals.c
pcre_info.c
pcre_internal.h
pcre_maketables.c
pcre_newline.c
pcre_ord2utf8.c
pcre_refcount.c
pcre_study.c
pcre_tables.c
pcre_try_flipped.c
pcre_ucd.c
pcre_valid_utf8.c
pcre_version.c
pcre_xclass.c
ucp.h

3. Edit: pcre_compile.c, pcre_dfa_exec.c, pcre_exec.c (individually) and  add:

#pragma warning(disable: 4018)

to the start of each file.