cd \source\mushclient\

echo Converting documentation file ...
lua5.1.exe generate_documentation.lua

echo Creating documentation help.db file ...
del help.db
sqlite3 help.db < documentation_fixed.sql
echo Vacuuming the database ...
sqlite3 help.db vacuum
echo Analyzing the database ...
sqlite3 help.db analyze

echo Building list of files to process ...
lua5.1.exe build_files_list.lua

echo Generating internationalization files ...
xgettext  -kTMessageBox -kTranslate -kTranslate_NoOp -o mushclient_static.po --files-from=all_files_list.txt
xgettext  -kTFormat -o mushclient_formatted.po --files-from=all_files_list.txt
xgettext  -kTranslateTime -o mushclient_time.po --files-from=all_files_list.txt
xgettext  -kTranslateHeading -o mushclient_heading.po --files-from=all_files_list.txt

echo Running localize.lua ...
lua5.1.exe localize.lua
