MUSHclient version 3.04
=======================

Release notes.

Sunday, 13 August 2000


Author: Nick Gammon <nick@gammon.com.au>

-----------------------------------------
If you are reading this file with NotePad,
enable "Word Wrap" under the Edit menu for
proper viewing of it.
-----------------------------------------

MUSHclient is produced by Gammon Software Solutions:
	http://www.gammon.com.au



INSTALLATION

You *only* need to download the file mushclient304.exe - this is a self-extracting installer.


CHANGES

1. Fixed bug where if you did a "world.setstatus" from within a script, the status line would not update until 5 seconds had elapsed.

2. Added a link to http://www.gammon.com.au/orders.htm on the shareware registration page, in case users experience problems registering MUSHclient.

3. Reworded the message: 
 
   Executing alias script "blah" 
     to
   Executing alias subroutine "blah" 
   
   Strictly speaking, it is not executing a script, but a subroutine inside the script. The same applies to triggers, timers, etc.

4. In the notepad, you can now toggle insert/overwrite mode with the Insert key. The current mode is shown on the status line.

5. When editing an alias, if you check "speed walk" then "queue" is greyed out, as speed walks are always queued.

6. Added script methods:
   WorldAddress - returns the TCP/IP address of the current world
   WorldPort - returns the port number of the current world

7. Added "ASCII Art" feature to the notepad. This lets you generate "big" letters like this:

 __  __ _   _ ____  _   _      _ _            _   
|  \/  | | | / ___|| | | | ___| (_) ___ _ __ | |_ 
| |\/| | | | \___ \| |_| |/ __| | |/ _ \ '_ \| __|
| |  | | |_| |___) |  _  | (__| | |  __/ | | | |_ 
|_|  |_|\___/|____/|_| |_|\___|_|_|\___|_| |_|\__|


  (this will only make sense if you are reading it in a monospaced font).

This will be handy for "welcome" screens for MUDs, and also is nifty in email signatures.
Various fonts are supplied in the download, and more are available from the FIGlet web site (the code to draw the letters is based on FIGlet).

8. Fixed a bug where, under some circumstances, if a trigger sent a message to the MUD, and also called a script, the wildcards supplied to the script would be incorrect.


  
Comments welcome
----------------

Please let me know if you have any problems. Check out the web pages mentioned above for details about later versions or known bugs.

Send mail to:
	Nick Gammon <nick@gammon.com.au>

