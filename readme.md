MUSHclient source

Published to GitHub on 21st. January 2010

Author: Nick Gammon

This software is released under the conditions in the file docs/agreement.txt


To compile:

Certain directories only have skeleton files. These are because the "real" files are in separate libraries which you can download for yourself.

These are:

* pcre
* png
* sqlite
* zlib

In each of those folders is a readme.txt file which explains where to get the relevant libraries, and any minor adjustments needed to make them work with MUSHclient.

You also need the following files:

* lua5.1.dll  (available from the normal MUSHclient download)
* locale/en.dll (put this under your WinRel or WinDebug directory)