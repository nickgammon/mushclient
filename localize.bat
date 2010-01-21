cd \source\mushclient\

xgettext  -kTMessageBox -kTranslate -kTranslate_NoOp -o mushclient_static.po *.cpp *.c
xgettext  -kTFormat -o mushclient_formatted.po *.cpp *.c
xgettext  -kTranslateTime -o mushclient_time.po *.cpp *.c
xgettext  -kTranslateHeading -o mushclient_heading.po *.cpp *.c

lua5.1.exe localize.lua
