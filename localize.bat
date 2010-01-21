cd \source\mushclient\

lua5.1.exe build_files_list.lua

xgettext  -kTMessageBox -kTranslate -kTranslate_NoOp -o mushclient_static.po --files-from=all_files_list.txt
xgettext  -kTFormat -o mushclient_formatted.po --files-from=all_files_list.txt
xgettext  -kTranslateTime -o mushclient_time.po --files-from=all_files_list.txt
xgettext  -kTranslateHeading -o mushclient_heading.po --files-from=all_files_list.txt

lua5.1.exe localize.lua
