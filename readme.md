MUSHclient source
=================

Published to GitHub on 21st. January 2010

Author: Nick Gammon

This software is released under the conditions in the file docs/agreement.txt


To compile
----------

Certain directories only have skeleton files. These are because the "real" files are in separate libraries which you can download for yourself.

These are:

* pcre
* png
* sqlite
* zlib

In each of those folders is a readme.txt file which explains where to get the relevant libraries, and any minor adjustments needed to make them work with MUSHclient.

To run
------

You need to copy various files from the standard MUSHclient distribution release in order to test the compiled code.

They are not part of the source distribution because they are not, strictly speaking, the "source" required to compile it.

These files should be copied into the place where MUSHclient.exe was put by the compiler. In my case this would be the WinRel or WinDebug directories.


*Files*

* lua5.1.dll  - for Lua scripting
* mushclient.hlp - help file
* mushclient.cnt - help table of contents
* tips.txt - tips shown at startup
* spellchecker.lua - Lua file that does the spell checking (loaded at startup)



*Directories*

* **locale** directory (this has the resources (menus, dialogs etc.) )
* **spell** directory (so the spell checker can find its dictionaries)
* **lua** directory (if you are planning to use any Lua modules like "tprint")
