; -------- MUSHclient install script
; For use with NSIS installer (see www.nullsort.com)

;
;  get a local copy of the .exe and compress it
;

;  !system "copy ..\WinRel\mushclient.exe mushclient.exe" = 0
;  !system "upx mushclient.exe" = 0

!system "..\localize.bat" = 0

; The name of the installer
Name "MUSHclient"
Caption "MUSHclient Installer"
; BGGradient FE8100 FCFC04  771F0E 
; InstallColors FF8080 000030


; The file to write
OutFile mushclient40x.exe

; The default installation directory
InstallDir $PROGRAMFILES\MUSHclient

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKCU "SOFTWARE\Gammon Software Solutions\MUSHclient\General Options" "InstallDir"

; The text to prompt the user to enter a directory
ComponentText "This will install MUSHclient on your computer. Select which optional components that you want installed."

; The text to prompt the user to enter a directory
DirText "Setup has determined the optimal location to install. If you would like to change the directory, do so now."

; options

CRCCheck on
ShowInstDetails show
SetOverwrite on
;SetOverwrite ifnewer
SetCompress auto
SetCompressor lzma
SetDateSave on

; license

LicenseText "Please read the following agreement and click on 'Next' if you agree to it:"
LicenseData "..\docs\agreement.txt"

; install types

InstType Typical  ; 1
InstType Full     ; 2
InstType Compact  ; 3
CheckBitmap  "${NSISDIR}\Contrib\Graphics\Checks\simple-round2.bmp" 

Function .onInit

; splash screen

# the plugins dir is automatically deleted when the installer exits
;     InitPluginsDir
;
;    File /oname=$PLUGINSDIR\splash.bmp "MUSHclient Installer.bmp"
;    splash::show 3000 $PLUGINSDIR\splash
; 
; Pop $0 

FunctionEnd


; The stuff to install
Section "-MUSHclient program (required)"
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Which section type it is in
  SectionIn 1 2 3  ; typical, full, compact

    WriteUninstaller "uninstall.exe"

  ; Put files there

  File "Readme.txt" ; in install directory

  File "..\WinRel\mushclient.exe"
  ; File "..\MUSHclient.exe.manifest"
  File "..\WinRel\lua5.1.dll"
  File "..\lua5.1.lib"
  File "..\mushclient.HLP"
  File "..\mushclient.cnt"
  File "..\install\mushclient.ico"
  File "..\tips.txt"
  File "..\Example_Filters.lua"
  File "..\names.txt"
  File /oname=license.txt "..\docs\agreement.txt"
  File "..\Dina.fon"

  ; Write the installation path into the registry
  WriteRegStr HKCU "Software\Gammon Software Solutions\MUSHclient\General Options" "InstallDir" $INSTDIR

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MUSHclient" "DisplayName" "MUSHclient (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MUSHclient" "UninstallString" "$INSTDIR\uninstall.exe"

  ; directory for world files (worlds subdirectory)

    ReadRegStr $1 HKCU "Software\Gammon Software Solutions\MUSHclient\Global prefs" "DefaultWorldFileDirectory"

    ; no existing directory - create one
    StrCmp $1 "" 0 NoCreateWorldDirectory
    WriteRegStr HKCU "Software\Gammon Software Solutions\MUSHclient\Global prefs" "DefaultWorldFileDirectory" "$INSTDIR\worlds\"
    CreateDirectory "$INSTDIR\worlds"

    ; not needed
    NoCreateWorldDirectory:

  
  ; directory for plugin files (worlds\plugins subdirectory)

    ReadRegStr $1 HKCU "Software\Gammon Software Solutions\MUSHclient\Global prefs" "PluginsDirectory"

    ; no existing directory - create one
    StrCmp $1 "" 0 NoCreatePluginsDirectory
    WriteRegStr HKCU "Software\Gammon Software Solutions\MUSHclient\Global prefs" "PluginsDirectory" "$INSTDIR\worlds\plugins\"
    CreateDirectory "$INSTDIR\worlds\plugins"
    CreateDirectory "$INSTDIR\worlds\plugins\state"

    ; not needed
    NoCreatePluginsDirectory:
      
  ; directory for log files (logs subdirectory)

    ReadRegStr $1 HKCU "Software\Gammon Software Solutions\MUSHclient\Global prefs" "DefaultLogFileDirectory"

    ; no existing directory - create one
    StrCmp $1 "" 0 NoCreateLogDirectory
    WriteRegStr HKCU "Software\Gammon Software Solutions\MUSHclient\Global prefs" "DefaultLogFileDirectory" "$INSTDIR\logs\"
    CreateDirectory "$INSTDIR\logs"

    ; not needed
    NoCreateLogDirectory:

  ;
  ;  directory for locale information
  ;
  CreateDirectory "$INSTDIR\locale"
  SetOutPath $INSTDIR\locale
   
  File "..\Localize_template.lua"
  File "..\WinRel\locale\en.dll"
  File "..\WinRel\locale\en_small.dll"
  File "..\count_locale_usage.lua"
  File "..\detect_locale_changes.lua"
  File "..\locale_notes.txt"
  
  SetOutPath $INSTDIR

  ;
  ;  put sounds here, hint hint
  ;
  CreateDirectory "$INSTDIR\sounds"
    
 ;
  ;  directory for name generation files
  ;
  CreateDirectory "$INSTDIR\names"
  SetOutPath $INSTDIR\names
   
  File "..\names\ALBION.NAM"
  File "..\names\ALVER.NAM"
  File "..\names\ALVER1.NAM"
  File "..\names\ALVER2.NAM"
  File "..\names\DEVERRY1.NAM"
  File "..\names\DEVERRY2.NAM"
  File "..\names\DVARGAR.NAM"
  File "..\names\Dragonl1.nam"
  File "..\names\Felana.nam"
  File "..\names\GALLER.NAM"
  File "..\names\HOBER.NAM"
  File "..\names\ORC1.NAM"
  File "..\names\ORC2.NAM"
  File "..\names\albion1.nam"
  File "..\names\albion2.nam"
  File "..\names\dvargar1.nam"
  File "..\names\dvargar2.nam"
  File "..\names\f_female.nam"
  File "..\names\f_male.nam"
  File "..\names\gnome1.nam"
  File "..\names\gnome2.nam"
  File "..\names\hober1.nam"
  File "..\names\hober2.nam"
  File "..\names\kender1.nam"
  File "..\names\kender2.nam"
  File "..\names\kerrel.nam"
  File "..\names\orc.nam"
  File "..\names\sparhawk.nam"
  
  SetOutPath $INSTDIR
    
SectionEnd

Section "Spell checker"
  ; Which section type it is in
  SectionIn 1 2   ; typical, full

  ; Set output path to the spell directory.
  SetOutPath $INSTDIR\spell

  ; Put files there

  File "..\spell\english-words.10"
  File "..\spell\english-words.20"
  File "..\spell\english-words.35"
  File "..\spell\english-words.40"
  File "..\spell\english-upper.10"
  File "..\spell\english-upper.35"
  File "..\spell\english-upper.40"
  File "..\spell\american-words.10"
  File "..\spell\american-words.20"
  File "..\spell\english-contractions.10"
  File "..\spell\english-contractions.35"
 

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; New spell checker
  File "..\WinRel\spellchecker.lua"

SectionEnd

Section "Documentation"
  ; Which section type it is in
  SectionIn 1 2   ; typical, full

  ; Set output path to the doc directory.
  SetOutPath $INSTDIR\docs

  ; Put files there
  File "..\docs\RegularExpressions.txt"
  File "..\docs\mersenne_twister.txt"
  File "..\docs\lua_license.txt"
  File "..\docs\PCRE_ChangeLog.txt"
  File "..\docs\gpl.txt"
  File "..\docs\lsqlite3.html"
  File "..\docs\lpeg.html"
  File "..\docs\re.html"
  File "..\docs\Lua Colors LICENSE.txt"
  
SectionEnd

Section "Fonts"

  ; Which section type it is in
  SectionIn 1 2   ; typical, full

  ; Set output path to the fonts directory.
  SetOutPath $INSTDIR\fonts

  ; Put files there
  File "..\fonts\banner.flf"
  File "..\fonts\big.flf"
  File "..\fonts\block.flf"
  File "..\fonts\bubble.flf"
  File "..\fonts\digital.flf"
  File "..\fonts\lean.flf"
  File "..\fonts\mini.flf"
  File "..\fonts\script.flf"
  File "..\fonts\shadow.flf"
  File "..\fonts\slant.flf"
  File "..\fonts\small.flf"
  File "..\fonts\smscript.flf"
  File "..\fonts\smshadow.flf"
  File "..\fonts\smslant.flf"
  File "..\fonts\standard.flf"
  File "..\fonts\term.flf"
SectionEnd

Section "Scripting"
  ; Which section type it is in
  SectionIn 1 2   ; typical, full

  ;
  ;  directory for scripts
  ;
  CreateDirectory "$INSTDIR\scripts"

  ;
  ;  directory for Lua scripts
  ;
  CreateDirectory "$INSTDIR\lua"
    
  ; Set output path to the lua subdirectory.
  SetOutPath $INSTDIR\lua

SetOverwrite ifnewer
   File "..\lua\addxml.lua"
   File "..\lua\check.lua"
   File "..\lua\declare.lua"
   File "..\lua\getlines.lua"
   File "..\lua\getstyle.lua"
   File "..\lua\pairsbykeys.lua"
   File "..\lua\serialize.lua"
   File "..\lua\tprint.lua"
   File "..\lua\var.lua"
   File "..\lua\wait.lua"
   File "..\lua\copytable.lua"
   File "..\lua\strict.lua"
   File "..\lua\commas.lua"
   File "..\lua\getworld.lua"
   File "..\lua\checkplugin.lua"
   File "..\lua\re.lua"
   File "..\lua\mw.lua"
   File "..\lua\movewindow.lua"
   File "..\lua\alphanum.lua"
   File "..\lua\InfoBox.lua" 
   File "..\lua\colors.lua"  
   File "..\lua\gauge.lua"  
 ; File "..\lua\ppi.lua"
   File "..\lua\mapper.lua"  

  ; Set output path to the scripts subdirectory.
  SetOutPath $INSTDIR\scripts

  ; Put files there
SetOverwrite ifnewer
  File "..\scripting\exampscript.vbs"
  File "..\scripting\exampscript.js"
  File "..\scripting\exampscript.pl"
  File "..\scripting\exampscript.pys"
  File "..\scripting\exampscript.lua"
SetOverwrite on
  File "..\WinRel\MUSHclient.tlb"
SectionEnd

Section "Plugins"
  ; Which section type it is in
  SectionIn 1 2   ; typical, full
  
  SetOutPath $INSTDIR\worlds\plugins

  ; Put files there
  File "..\plugins\constants.vbs"
  File "..\plugins\constants.js"
  File "..\plugins\constants.pl"
  File "..\plugins\constants.pys"
  File "..\plugins\constants.lua"
  File "..\plugins\plugin_list.xml"
  File "..\plugins\plugin_summary.xml"
  File "..\plugins\random_socials.xml"
  File "..\plugins\sample_plugin.xml"
  File "..\plugins\scan.xml"
  File "..\plugins\msp.xml"
  File "..\plugins\reverse_speedwalk.xml"
  File "..\plugins\SMAUG_automapper_helper.xml"
  File "..\plugins\multiple_send.xml"
  File "..\plugins\idle_message.xml"
  File "..\plugins\MUSH_teleport.xml"
  File "..\plugins\ColourNote.xml"
  File "..\plugins\NewActivity.xml"
  File "..\plugins\MudDatabase.xml"
  File "..\plugins\Chat.xml"
  File "..\plugins\Health_Bar.xml"
  File "..\plugins\JScript_Version.xml"
  File "..\plugins\Perlscript_Version.xml"
  File "..\plugins\Python_Version.xml"
  File "..\plugins\VBscript_Version.xml"
  File "..\plugins\ANSI_Log.xml"
  File "..\plugins\Hyperlink_URL.xml"
  File "..\plugins\Gag.xml"
  File "..\plugins\Status_Bar_Prompt.xml"
  File "..\plugins\Timer.xml"
  File "..\plugins\ShowActivity.xml"
  File "..\plugins\Installer_sumcheck.xml"
  File "..\plugins\lua_chat.xml"
  File "..\plugins\Current_Output_Window.xml"
  File "..\plugins\InfoBox_Demo.xml"
  File "..\plugins\ATCP_NJG.xml"


SectionEnd


; optional section
Section "Start Menu Shortcuts"
  ; Which section type it is in
  SectionIn 1 2   ; typical, full

;  SetOutPath $INSTDIR
  SetOutPath "$SMPROGRAMS\MUSHclient"
  CreateShortCut "$SMPROGRAMS\MUSHclient\MUSHclient.lnk" "$INSTDIR\MUSHclient.exe" "" "$INSTDIR\MUSHclient.exe" 0
  CreateShortCut "$SMPROGRAMS\MUSHclient\Read Me.lnk" "$INSTDIR\Readme.txt" "" "$INSTDIR\Readme.txt" 0
  CreateShortCut "$SMPROGRAMS\MUSHclient\Uninstall MUSHclient.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

; uninstall stuff
UninstallText "Are you sure you want to remove MUSHclient and all of its components?"

; special uninstall section.
Section Uninstall

  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MUSHclient"
  DeleteRegKey HKCU "Software\Gammon Software Solutions\MUSHclient"

  ; remove files

  ; program
  Delete "$INSTDIR\MUSHclient.exe"
  ; Delete "$INSTDIR\MUSHclient.exe.manifest"
  Delete "$INSTDIR\StatusBar.exe"
  Delete "$INSTDIR\lua5.1.dll"
  Delete "$INSTDIR\lua5.1.lib"
  Delete "$INSTDIR\MUSHclient.ico"
  Delete "$INSTDIR\MUSHclient.hlp"
  Delete "$INSTDIR\MUSHclient.cnt"
  Delete "$INSTDIR\license.txt"

  ; readme and tips
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\tips.txt"
  
  ; example trigger/alias/timer list filters
  Delete "$INSTDIR\Example_Filters.lua"

  ; and character name generation file
  Delete "$INSTDIR\names.txt"

  ; and Dina font
  Delete "$INSTDIR\Dina.fon"
  
  ; scripting stuff
  Delete "$INSTDIR\scripts\MUSHclient.tlb"
  Delete "$INSTDIR\scripts\exampscript.vbs"
  Delete "$INSTDIR\scripts\exampscript.js"
  Delete "$INSTDIR\scripts\exampscript.pl"
  Delete "$INSTDIR\scripts\exampscript.pys"
  Delete "$INSTDIR\scripts\exampscript.lua"

  RMDir  "$INSTDIR\scripts"
  
  ; lua stuff
  Delete "$INSTDIR\lua\addxml.lua"
  Delete "$INSTDIR\lua\check.lua"
  Delete "$INSTDIR\lua\declare.lua"
  Delete "$INSTDIR\lua\getlines.lua"
  Delete "$INSTDIR\lua\getstyle.lua"
  Delete "$INSTDIR\lua\pairsbykeys.lua"
  Delete "$INSTDIR\lua\serialize.lua"
  Delete "$INSTDIR\lua\tprint.lua"
  Delete "$INSTDIR\lua\var.lua"
  Delete "$INSTDIR\lua\wait.lua"
  Delete "$INSTDIR\lua\copytable.lua"
  Delete "$INSTDIR\lua\strict.lua"
  Delete "$INSTDIR\lua\commas.lua"
  Delete "$INSTDIR\lua\getworld.lua"
  Delete "$INSTDIR\lua\checkplugin.lua" 
  Delete "$INSTDIR\lua\re.lua" 
  Delete "$INSTDIR\lua\mw.lua" 
  Delete "$INSTDIR\lua\movewindow.lua" 
  Delete "$INSTDIR\lua\alphanum.lua" 
  Delete "$INSTDIR\lua\InfoBox.lua" 
  Delete "$INSTDIR\lua\colors.lua" 
  Delete "$INSTDIR\lua\gauge.lua" 
; Delete "$INSTDIR\lua\ppi.lua"
  Delete "$INSTDIR\lua\mapper.lua"  
  
  ; spell checker stuff
  Delete "$INSTDIR\spellchecker.lua"
  Delete "$INSTDIR\spell\english-words.10"
  Delete "$INSTDIR\spell\english-words.20"
  Delete "$INSTDIR\spell\english-words.35"
  Delete "$INSTDIR\spell\english-words.40"
  Delete "$INSTDIR\spell\english-upper.10"
  Delete "$INSTDIR\spell\english-upper.35"
  Delete "$INSTDIR\spell\english-upper.40"
  Delete "$INSTDIR\spell\american-words.10"
  Delete "$INSTDIR\spell\american-words.20"
  Delete "$INSTDIR\spell\english-contractions.10"
  Delete "$INSTDIR\spell\english-contractions.35"
  
  RMDir  "$INSTDIR\spell"

  ; docs
  Delete "$INSTDIR\docs\RegularExpressions.txt"
  Delete "$INSTDIR\docs\mersenne_twister.txt"
  Delete "$INSTDIR\docs\lua_license.txt"
  Delete "$INSTDIR\docs\PCRE_ChangeLog.txt"
  Delete "$INSTDIR\docs\gpl.txt"
  Delete "$INSTDIR\docs\lsqlite3.html"
  Delete "$INSTDIR\docs\lpeg.html"
  Delete "$INSTDIR\docs\re.html"
  Delete "$INSTDIR\docs\Lua Colors LICENSE.txt"
  
  RMDir  "$INSTDIR\docs"

  ; fonts
  Delete "$INSTDIR\fonts\*.flf"
  RMDir  "$INSTDIR\fonts"

  ; plugins
  Delete "$INSTDIR\worlds\plugins\constants.vbs"
  Delete "$INSTDIR\worlds\plugins\constants.js"
  Delete "$INSTDIR\worlds\plugins\constants.pl"
  Delete "$INSTDIR\worlds\plugins\constants.pys"
  Delete "$INSTDIR\worlds\plugins\constants.lua"
  Delete "$INSTDIR\worlds\plugins\plugin_list.xml"
  Delete "$INSTDIR\worlds\plugins\random_socials.xml"
  Delete "$INSTDIR\worlds\plugins\sample_plugin.xml"
  Delete "$INSTDIR\worlds\plugins\scan.xml"
  Delete "$INSTDIR\worlds\plugins\msp.xml"
  Delete "$INSTDIR\worlds\plugins\reverse_speedwalk.xml"
  Delete "$INSTDIR\worlds\plugins\SMAUG_automapper_helper.xml"
  Delete "$INSTDIR\worlds\plugins\multiple_send.xml"
  Delete "$INSTDIR\worlds\plugins\idle_message.xml"
  Delete "$INSTDIR\worlds\plugins\MUSH_teleport.xml"
  Delete "$INSTDIR\worlds\plugins\ColourNote.xml"
  Delete "$INSTDIR\worlds\plugins\NewActivity.xml"
  Delete "$INSTDIR\worlds\plugins\MudDatabase.xml"
  Delete "$INSTDIR\worlds\plugins\Chat.xml"
  Delete "$INSTDIR\worlds\plugins\Health_Bar.xml"
  Delete "$INSTDIR\worlds\plugins\JScript_Version.xml"
  Delete "$INSTDIR\worlds\plugins\Perlscript_Version.xml"
  Delete "$INSTDIR\worlds\plugins\Python_Version.xml"
  Delete "$INSTDIR\worlds\plugins\VBscript_Version.xml"
  Delete "$INSTDIR\worlds\plugins\ANSI_Log.xml"
  Delete "$INSTDIR\worlds\plugins\Hyperlink_URL.xml"
  Delete "$INSTDIR\worlds\plugins\Gag.xml"
  Delete "$INSTDIR\worlds\plugins\Status_Bar_Prompt.xml"
  Delete "$INSTDIR\worlds\plugins\Timer.xml"
  Delete "$INSTDIR\worlds\plugins\ShowActivity.xml"
  Delete "$INSTDIR\worlds\plugins\Installer_sumcheck.xml"
  Delete "$INSTDIR\worlds\plugins\lua_chat.xml"
  Delete "$INSTDIR\worlds\plugins\Current_Output_Window.xml"
  Delete "$INSTDIR\worlds\plugins\InfoBox_Demo.xml"
  Delete "$INSTDIR\worlds\plugins\ATCP_NJG.xml"
  
  ; locale stuff
  
  Delete "$INSTDIR\locale\Localize_template.lua"
  Delete "$INSTDIR\locale\en.dll"
  Delete "$INSTDIR\locale\en_small.dll"
  Delete "$INSTDIR\locale\count_locale_usage.lua"
  Delete "$INSTDIR\locale\detect_locale_changes.lua"
  Delete "$INSTDIR\locale\locale_notes.txt"
  
   ; names files
   
  Delete "$INSTDIR\names\ALBION.NAM"
  Delete "$INSTDIR\names\ALVER.NAM"
  Delete "$INSTDIR\names\ALVER1.NAM"
  Delete "$INSTDIR\names\ALVER2.NAM"
  Delete "$INSTDIR\names\DEVERRY1.NAM"
  Delete "$INSTDIR\names\DEVERRY2.NAM"
  Delete "$INSTDIR\names\DVARGAR.NAM"
  Delete "$INSTDIR\names\Dragonl1.nam"
  Delete "$INSTDIR\names\Felana.nam"
  Delete "$INSTDIR\names\GALLER.NAM"
  Delete "$INSTDIR\names\HOBER.NAM"
  Delete "$INSTDIR\names\ORC1.NAM"
  Delete "$INSTDIR\names\ORC2.NAM"
  Delete "$INSTDIR\names\albion1.nam"
  Delete "$INSTDIR\names\albion2.nam"
  Delete "$INSTDIR\names\dvargar1.nam"
  Delete "$INSTDIR\names\dvargar2.nam"
  Delete "$INSTDIR\names\f_female.nam"
  Delete "$INSTDIR\names\f_male.nam"
  Delete "$INSTDIR\names\gnome1.nam"
  Delete "$INSTDIR\names\gnome2.nam"
  Delete "$INSTDIR\names\hober1.nam"
  Delete "$INSTDIR\names\hober2.nam"
  Delete "$INSTDIR\names\kender1.nam"
  Delete "$INSTDIR\names\kender2.nam"
  Delete "$INSTDIR\names\kerrel.nam"
  Delete "$INSTDIR\names\orc.nam"
  Delete "$INSTDIR\names\sparhawk.nam"
    
  RMDir  "$INSTDIR\names"
  RMDir  "$INSTDIR\sounds"
  
  ; MUST REMOVE UNINSTALLER, too
  Delete $INSTDIR\uninstall.exe

  ; remove shortcuts, if any.
  Delete "$SMPROGRAMS\MUSHclient\*.lnk"
  RMDir "$SMPROGRAMS\MUSHclient"

  ; delete worlds, lua and logs directory, if empty
  RMDir  "$INSTDIR\worlds"
  RMDir  "$INSTDIR\logs"
  RMDir  "$INSTDIR\lua"
  RMDir  "$INSTDIR\locale"

  ; remove directories used.
  RMDir "$INSTDIR"

  ; warn user if not all files removed
  IfErrors CannotRemoveDirectory CanRemoveDirectory

  CannotRemoveDirectory:

  MessageBox MB_OK|MB_ICONEXCLAMATION  \
    "Some files or folders (probably that you created) could not be removed and should be deleted manually if desired."

  CanRemoveDirectory:

SectionEnd

; post-install

Function .onInstSuccess
;  MessageBox MB_YESNO "Installation of MUSHclient is complete. View the readme file?" IDNO NoReadme
;    Exec 'notepad "$INSTDIR\readme.txt"' 
;  NoReadme:
  
  MessageBox MB_YESNO "Launch MUSHclient now?" IDNO NoLaunch
    SetOutPath $INSTDIR
    ExecShell "open" "$INSTDIR\mushclient.exe"
  NoLaunch:
FunctionEnd


