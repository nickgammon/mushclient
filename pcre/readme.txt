PCRE built as follows:

1. Download: http://sourceforge.net/projects/pcre/files/pcre/8.10/pcre-8.10.zip/download

2. Unzip the file pcre-8.10.zip


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


(Note that config.h is part of the git repository)

4. Rename pcre_chartables.c.dist as pcre_chartables.c
   Rename pcre.h.generic as pcre.h


5. Edit: pcre_compile.c and add:

#pragma warning( disable : 4018)  // '<' : signed/unsigned mismatch
#pragma warning( disable : 4244)  // conversion from 'int' to 'unsigned short', possible loss of data
#pragma warning( disable : 4701)  // local variable 'othercase' may be used without having been initialized

to the start of the file.	


6. Edit all of: 

  pcre_dfa_exec.c
  pcre_exec.c
  pcre_maketables.c
  pcre_ord2utf8.c
  pcre_refcount.c
  pcre_study.c

and add:
 
#pragma warning( disable : 4244)  // conversion from 'int' to 'unsigned short', possible loss of data

to the start of each file.	


