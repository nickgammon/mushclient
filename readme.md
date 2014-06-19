MUSHclient source
=================

Published to GitHub on 21st. January 2010

Author: Nick Gammon

This software is released under the conditions in the file docs/agreement.txt


To compile
----------

Certain directories only have skeleton files. These are because the "real" files are in separate libraries which you can download for yourself.

These libraries are (along with their home pages):

* pcre	  - <http://www.pcre.org/>
* png	  - <http://www.libpng.org/pub/png/libpng.html>
* sqlite  - <http://www.sqlite.org/>
* zlib	  - <http://www.zlib.net/>

In each of those folders is a **readme.txt** file which explains where to get the relevant libraries, and any minor adjustments needed to make them work with MUSHclient.

To run
------

You need to copy various files from the standard MUSHclient distribution release in order to test the compiled code.

They are not part of the source distribution because they are not, strictly speaking, the "source" required to compile it.

These files should be copied into the place where MUSHclient.exe was put by the compiler. In my case this would be the WinRel or WinDebug directories.

* Files

	* lua5.1.dll  - for Lua scripting
	* mushclient.hlp - help file
	* mushclient.cnt - help table of contents
	* tips.txt - tips shown at startup
	* spellchecker.lua - Lua file that does the spell checking (loaded at startup)

* Directories

	* **locale** directory (this has the resources file en.dll which contains menus, dialogs etc.)
	* **spell** directory (so the spell checker can find its dictionaries)
	* **lua** directory (if you are planning to use any Lua modules like "tprint")

Resources
---------

The resources used by MUSHclient are in the file en.dll (which resides in the directory "locale" mentioned above). To rebuild file you need to download the project	mushclient_resources which is on GitHub:

<https://github.com/nickgammon/mushclient_resources>