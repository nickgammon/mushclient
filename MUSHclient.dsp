# Microsoft Developer Studio Project File - Name="MUSHclient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=MUSHclient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MUSHclient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MUSHclient.mak" CFG="MUSHclient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MUSHclient - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "MUSHclient - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Windows/MUSHclient", FAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\WinDebug"
# PROP BASE Intermediate_Dir ".\WinDebug"
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\WinDebug"
# PROP Intermediate_Dir ".\WinDebug"
# PROP Ignore_Export_Lib 0
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MTd /W4 /Gm /Gi /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "LUA51" /Fr /Yu"stdafx.h" /FD /c
# ADD MTL /nologo /D "_DEBUG" /mktyplib203
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 dsound.lib Shlwapi.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\WinRel"
# PROP BASE Intermediate_Dir ".\WinRel"
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\WinRel"
# PROP Intermediate_Dir ".\WinRel"
# PROP Ignore_Export_Lib 0
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "ZLIB_DLL" /D "LUA51" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 dsound.lib Shlwapi.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none /debug

!ENDIF 

# Begin Target

# Name "MUSHclient - Win32 Debug"
# Name "MUSHclient - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\accelerators.cpp
# End Source File
# Begin Source File

SOURCE=.\ActivityDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\ansi.cpp
# End Source File
# Begin Source File

SOURCE=.\art.cpp
# End Source File
# Begin Source File

SOURCE=.\chatlistensock.cpp
# End Source File
# Begin Source File

SOURCE=.\CHATSOCK.CPP
# End Source File
# Begin Source File

SOURCE=.\Color.cpp
# End Source File
# Begin Source File

SOURCE=.\DDV_validation.cpp
# End Source File
# Begin Source File

SOURCE=.\Dmetaph.cpp
# End Source File
# Begin Source File

SOURCE=.\doc.cpp
# End Source File
# Begin Source File

SOURCE=.\doc_construct.cpp
# End Source File
# Begin Source File

SOURCE=.\evaluate.cpp
# End Source File
# Begin Source File

SOURCE=.\exceptions.cpp
# End Source File
# Begin Source File

SOURCE=.\Finding.cpp
# End Source File
# Begin Source File

SOURCE=.\format.cpp
# End Source File
# Begin Source File

SOURCE=.\genprint.cpp
# End Source File
# Begin Source File

SOURCE=.\globalregistryoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\HyprLink.cpp
# End Source File
# Begin Source File

SOURCE=.\Line.cpp
# End Source File
# Begin Source File

SOURCE=.\Mapping.cpp
# End Source File
# Begin Source File

SOURCE=.\mcdatetime.cpp
# End Source File
# Begin Source File

SOURCE=.\MiniWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\MUSHclient.cpp
# End Source File
# Begin Source File

SOURCE=.\MUSHclient.odl
# End Source File
# Begin Source File

SOURCE=.\MUSHclient.rc
# End Source File
# Begin Source File

SOURCE=.\MyStatusBar.cpp
# End Source File
# Begin Source File

SOURCE=.\NameGeneration.cpp
# End Source File
# Begin Source File

SOURCE=.\plugins.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcessPreviousLine.cpp
# End Source File
# Begin Source File

SOURCE=.\regexp.cpp
# End Source File
# Begin Source File

SOURCE=.\Replace.cpp
# End Source File
# Begin Source File

SOURCE=.\serialize.cpp
# End Source File
# Begin Source File

SOURCE=.\StatLink.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\telnet_phases.cpp
# End Source File
# Begin Source File

SOURCE=.\TextDocument.cpp
# End Source File
# Begin Source File

SOURCE=.\TimerWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\UDPsocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Utilities.cpp
# End Source File
# Begin Source File

SOURCE=.\winplace.cpp
# End Source File
# Begin Source File

SOURCE=.\world_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\worldsock.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\dialogs\global_prefs\GlobalPrefsSheet.h
# End Source File
# Begin Source File

SOURCE=.\luacom\LuaAux.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\LuaAux.h
# End Source File
# Begin Source File

SOURCE=.\luacom\luabeans.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\luabeans.h
# End Source File
# Begin Source File

SOURCE=.\luacom\luacom_internal.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\luacom_internal.h
# End Source File
# Begin Source File

SOURCE=.\luacom\LuaCompat.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\LuaCompat.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tCOMUtil.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tCOMUtil.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaCOM.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOM.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaCOMClassFactory.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOMClassFactory.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaCOMConnPoints.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOMConnPoints.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaCOMEnumerator.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOMEnumerator.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaCOMException.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOMException.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaCOMTypeHandler.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOMTypeHandler.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaControl.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaControl.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaDispatch.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaDispatch.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaObject.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaObject.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaObjList.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaObjList.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaTLB.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaTLB.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tLuaVector.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaVector.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tStringBuffer.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tStringBuffer.h
# End Source File
# Begin Source File

SOURCE=.\luacom\src\library\tUtil.h
# End Source File
# Begin Source File

SOURCE=.\luacom\tUtil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\doc.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_main.ico
# End Source File
# Begin Source File

SOURCE=.\res\MUSHclient.ico
# End Source File
# End Group
# Begin Group "Prefs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\scriptingoptions.cpp
# End Source File
# End Group
# Begin Group "Dialogs"

# PROP Default_Filter ""
# Begin Group "chat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dialogs\chat\ChatCallDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\chat\ChatDetailsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\chat\ChatListDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\chat\ChatToAllDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\chat\EmoteToAllDlg.cpp
# End Source File
# End Group
# Begin Group "global_prefs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dialogs\global_prefs\GlobalPrefs.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\global_prefs\GlobalPrefsSheet.cpp
# End Source File
# End Group
# Begin Group "world_prefs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dialogs\world_prefs\aliasdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\ButtonColour.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\CommandOptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\configuration.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\editmac.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\EditVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\genpropertypage.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\MissingEntryPoints.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\prefspropertypages.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\PrefsPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\TabDefaultsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\TimerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\TreePropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\world_prefs\triggdlg.cpp
# End Source File
# End Group
# Begin Group "plugins"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dialogs\plugins\PluginsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\plugins\PluginWizard.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\plugins\PluginWizardSheet.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\dialogs\AsciiArtDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\ChooseNotepadDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\cmdhist.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\ColourComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\ColourPickerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\CompleteWordDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\ConfirmPreamble.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\CreditsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\DebugLuaDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\DebugWorldInputDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\EditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\EditMultiLine.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\FindDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\FunctionListDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\GeneratedNameDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\GlobalChangeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\GoToLineDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\HighlightPhraseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\ImmediateDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\ImportXMLdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\InsertUnicodeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\KeyNameDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\logdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\LuaChooseBox.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\LuaChooseList.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\LuaChooseListMulti.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\LuaGsubDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\LuaInputBox.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\LuaInputEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\MapCommentDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\MapDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\MapMoveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\MultiLineTriggerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\MXPscriptRoutinesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\password.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\ProgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\ProxyServerPasswordDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\QuickConnectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\RecallDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\RecallSearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\RegexpProblemDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\ScriptErrorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\SendToAllDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\SpellCheckDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\Splash.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\TextAttributesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\TipDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\UniqueIDDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\welcome.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\welcome1.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogs\WinsockInfoDlg.cpp
# End Source File
# End Group
# Begin Group "Views"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\activitychildfrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ActivityView.cpp
# End Source File
# Begin Source File

SOURCE=.\childfrm.cpp
# End Source File
# Begin Source File

SOURCE=.\mainfrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MakeWindowTransparent.cpp
# End Source File
# Begin Source File

SOURCE=.\MDITabs.cpp
# End Source File
# Begin Source File

SOURCE=.\mushview.cpp
# End Source File
# Begin Source File

SOURCE=.\MySplitterWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\sendvw.cpp
# End Source File
# Begin Source File

SOURCE=.\textchildfrm.cpp
# End Source File
# Begin Source File

SOURCE=.\TextView.cpp
# End Source File
# End Group
# Begin Group "MXP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mxp\mxp.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxp_phases.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpClose.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpCloseAtomic.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpEnd.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpEntities.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpError.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpinit.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpMode.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpOnOff.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpOpenAtomic.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxpStart.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp\mxputils.cpp
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zlib\adler32.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\compress.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\crc32.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\deflate.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\gzclose.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\gzlib.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\gzread.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\gzwrite.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\inffast.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\inflate.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\inftrees.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\minigzip.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\readme.txt
# End Source File
# Begin Source File

SOURCE=.\zlib\trees.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\uncompr.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\zlib\zutil.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "XML"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xml\xml_load_world.cpp
# End Source File
# Begin Source File

SOURCE=.\xml\xml_save_world.cpp
# End Source File
# Begin Source File

SOURCE=.\xml\xml_serialize.cpp
# End Source File
# Begin Source File

SOURCE=.\xml\xmlparse.cpp
# End Source File
# End Group
# Begin Group "Panes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\paneline.cpp
# End Source File
# End Group
# Begin Group "scripting"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\scripting\bits.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\scripting\functionlist.cpp
# End Source File
# Begin Source File

SOURCE=.\scripting\lbc.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\scripting\lpeg.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\scripting\lrexlib.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\scripting\lua_compress.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\scripting\lua_methods.cpp
# End Source File
# Begin Source File

SOURCE=.\scripting\lua_progressdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\scripting\lua_scripting.cpp
# End Source File
# Begin Source File

SOURCE=.\scripting\lua_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\scripting\md5.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\scripting\methods.cpp
# End Source File
# Begin Source File

SOURCE=.\scripting\mt19937ar.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\scripting\number.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\scripting\scriptengine.cpp
# End Source File
# Begin Source File

SOURCE=.\scripting\scripting.cpp
# End Source File
# Begin Source File

SOURCE=.\scripting\sha256.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\scripting\shs.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "pcre"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcre\pcre_chartables.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_compile.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_config.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_dfa_exec.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_exec.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_fullinfo.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_get.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_globals.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_info.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_maketables.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_newline.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_ord2utf8.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_refcount.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_study.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_tables.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_try_flipped.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_ucd.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_valid_utf8.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_version.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre_xclass.c
# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pcre\readme.txt
# End Source File
# End Group
# Begin Group "PNG"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\png\png.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngerror.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngget.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngmem.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngpread.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngread.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngrio.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngrtran.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngrutil.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngset.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngtrans.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngwio.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngwrite.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngwtran.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\pngwutil.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\png\readme.txt
# End Source File
# End Group
# Begin Group "sqlite3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sqlite3\readme.txt
# End Source File
# Begin Source File

SOURCE=.\sqlite3\sqlite3.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "lsqlite"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\lsqlite\lsqlite3.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "luacom"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\luacom\LuaAux.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\luabeans.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\luacom.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\LuaCompat.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tCOMUtil.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOM.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOMClassFactory.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOMConnPoints.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOMEnumerator.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOMException.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaCOMTypeHandler.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaControl.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaDispatch.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaObject.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaObjList.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaTLB.cpp
# ADD CPP /GR
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tLuaVector.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tStringBuffer.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\luacom\tUtil.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Source File

SOURCE=.\.gitignore
# End Source File
# Begin Source File

SOURCE=.\how_to_release.txt
# End Source File
# Begin Source File

SOURCE=.\install\readme.txt
# End Source File
# End Target
# End Project
# Section MUSHclient : {00000000-0001-0000-0000-000000000000}
# 	1:23:CG_IDR_POPUP_MAIN_FRAME:126
# End Section
# Section MUSHclient : {F3B13065-16CC-11D0-B7D0-00A0247B3BFD}
# 	0:8:TipDlg.h:D:\SOURCE\MUSHclient\TipDlg.h
# 	0:10:TipDlg.cpp:D:\SOURCE\MUSHclient\TipDlg.cpp
# 	1:17:CG_IDS_DIDYOUKNOW:108
# 	1:22:CG_IDS_TIPOFTHEDAYMENU:107
# 	1:18:CG_IDS_TIPOFTHEDAY:106
# 	1:22:CG_IDS_TIPOFTHEDAYHELP:111
# 	1:19:CG_IDP_FILE_CORRUPT:110
# 	1:7:IDD_TIP:105
# 	1:13:IDB_LIGHTBULB:103
# 	1:18:CG_IDS_FILE_ABSENT:109
# 	2:17:CG_IDS_DIDYOUKNOW:CG_IDS_DIDYOUKNOW
# 	2:7:CTipDlg:CTipDlg
# 	2:22:CG_IDS_TIPOFTHEDAYMENU:CG_IDS_TIPOFTHEDAYMENU
# 	2:18:CG_IDS_TIPOFTHEDAY:CG_IDS_TIPOFTHEDAY
# 	2:12:CTIP_Written:OK
# 	2:22:CG_IDS_TIPOFTHEDAYHELP:CG_IDS_TIPOFTHEDAYHELP
# 	2:2:BH:
# 	2:19:CG_IDP_FILE_CORRUPT:CG_IDP_FILE_CORRUPT
# 	2:7:IDD_TIP:IDD_TIP
# 	2:8:TipDlg.h:TipDlg.h
# 	2:13:IDB_LIGHTBULB:IDB_LIGHTBULB
# 	2:18:CG_IDS_FILE_ABSENT:CG_IDS_FILE_ABSENT
# 	2:10:TipDlg.cpp:TipDlg.cpp
# End Section
# Section MUSHclient : {F3B1306C-16CC-11D0-B7D0-00A0247B3BFD}
# 	0:8:Splash.h:D:\SOURCE\MUSHclient\Splash.h
# 	0:10:Splash.cpp:D:\SOURCE\MUSHclient\Splash.cpp
# 	1:10:IDB_SPLASH:102
# 	2:10:ResHdrName:resource.h
# 	2:11:ProjHdrName:stdafx.h
# 	2:10:WrapperDef:_SPLASH_SCRN_
# 	2:12:SplClassName:CSplashWnd
# 	2:21:SplashScreenInsertKey:4.0
# 	2:10:HeaderName:Splash.h
# 	2:10:ImplemName:Splash.cpp
# 	2:7:BmpID16:IDB_SPLASH
# End Section
# Section MUSHclient : {00000000-0000-0000-0000-000000000000}
# 	1:22:CG_IDR_POPUP_SEND_VIEW:124
# End Section
# Section MUSHclient : {52A03A14-D2CC-0056-A8D2-560020D55600}
# 	1:21:CG_IDR_POPUP_MUSHVIEW:125
# End Section
# Section MUSHclient : {00000000-000C-0000-0C00-00004944445F}
# 	1:26:CG_IDR_POPUP_ACTIVITY_VIEW:127
# End Section
