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
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "ZLIB_DLL" /D "LUA51" /Fr /Yu"stdafx.h" /FD /c
# ADD MTL /nologo /D "_DEBUG" /mktyplib203
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 dsound.lib /nologo /subsystem:windows /debug /machine:I386
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
# ADD LINK32 dsound.lib /nologo /subsystem:windows /machine:I386
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

SOURCE=.\documentation.cpp
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

SOURCE=.\ProcessPreviousLine.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgDlg.cpp
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

SOURCE=.\shs.c
# SUBTRACT CPP /YX /Yc /Yu
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

SOURCE=.\activitychildfrm.h
# End Source File
# Begin Source File

SOURCE=.\ActivityDoc.h
# End Source File
# Begin Source File

SOURCE=.\ActivityView.h
# End Source File
# Begin Source File

SOURCE=.\aliasdlg.h
# End Source File
# Begin Source File

SOURCE=.\AsciiArtDlg.h
# End Source File
# Begin Source File

SOURCE=.\ButtonColour.h
# End Source File
# Begin Source File

SOURCE=.\ChatCallDlg.h
# End Source File
# Begin Source File

SOURCE=.\ChatDetailsDlg.h
# End Source File
# Begin Source File

SOURCE=.\ChatListDlg.h
# End Source File
# Begin Source File

SOURCE=.\CHATSOCK.H
# End Source File
# Begin Source File

SOURCE=.\ChatToAllDlg.h
# End Source File
# Begin Source File

SOURCE=.\childfrm.h
# End Source File
# Begin Source File

SOURCE=.\ChooseNotepadDlg.h
# End Source File
# Begin Source File

SOURCE=.\cmdhist.h
# End Source File
# Begin Source File

SOURCE=.\ColourComboBox.h
# End Source File
# Begin Source File

SOURCE=.\ColourPickerDlg.h
# End Source File
# Begin Source File

SOURCE=.\CommandOptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\CompleteWordDlg.h
# End Source File
# Begin Source File

SOURCE=.\ConfirmPreamble.h
# End Source File
# Begin Source File

SOURCE=.\connectd.h
# End Source File
# Begin Source File

SOURCE=.\CreditsDlg.h
# End Source File
# Begin Source File

SOURCE=.\CWorldPrefs.h
# End Source File
# Begin Source File

SOURCE=.\CWorldPrefsSheet.h
# End Source File
# Begin Source File

SOURCE=.\DebugLuaDlg.h
# End Source File
# Begin Source File

SOURCE=.\DebugWorldInputDlg.h
# End Source File
# Begin Source File

SOURCE=.\DelayDlg.h
# End Source File
# Begin Source File

SOURCE=.\doc.h
# End Source File
# Begin Source File

SOURCE=.\EasterEggDlg.h
# End Source File
# Begin Source File

SOURCE=.\EditDlg.h
# End Source File
# Begin Source File

SOURCE=.\editmac.h
# End Source File
# Begin Source File

SOURCE=.\EditMultiLine.h
# End Source File
# Begin Source File

SOURCE=.\editstr.h
# End Source File
# Begin Source File

SOURCE=.\EditVariable.h
# End Source File
# Begin Source File

SOURCE=.\EmoteToAllDlg.h
# End Source File
# Begin Source File

SOURCE=.\errors.h
# End Source File
# Begin Source File

SOURCE=.\FindDlg.h
# End Source File
# Begin Source File

SOURCE=.\flags.h
# End Source File
# Begin Source File

SOURCE=.\FunctionListDlg.h
# End Source File
# Begin Source File

SOURCE=.\GeneratedNameDlg.h
# End Source File
# Begin Source File

SOURCE=.\genprint.h
# End Source File
# Begin Source File

SOURCE=.\GlobalChangeDlg.h
# End Source File
# Begin Source File

SOURCE=.\HighlightPhraseDlg.h
# End Source File
# Begin Source File

SOURCE=.\hostsite.h
# End Source File
# Begin Source File

SOURCE=.\ImmediateDlg.h
# End Source File
# Begin Source File

SOURCE=.\ImportXMLdlg.h
# End Source File
# Begin Source File

SOURCE=.\InsertUnicodeDlg.h
# End Source File
# Begin Source File

SOURCE=.\KeyNameDlg.h
# End Source File
# Begin Source File

SOURCE=.\logdlg.h
# End Source File
# Begin Source File

SOURCE=.\lua\lstate.h
# End Source File
# Begin Source File

SOURCE=.\lua_helpers.h
# End Source File
# Begin Source File

SOURCE=.\LuaChooseBox.h
# End Source File
# Begin Source File

SOURCE=.\LuaChooseList.h
# End Source File
# Begin Source File

SOURCE=.\LuaChooseListMulti.h
# End Source File
# Begin Source File

SOURCE=.\LuaGsubDlg.h
# End Source File
# Begin Source File

SOURCE=.\LuaInputBox.h
# End Source File
# Begin Source File

SOURCE=.\LuaInputEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\mainfrm.h
# End Source File
# Begin Source File

SOURCE=.\MapCommentDlg.h
# End Source File
# Begin Source File

SOURCE=.\MapDlg.h
# End Source File
# Begin Source File

SOURCE=.\MapMoveDlg.h
# End Source File
# Begin Source File

SOURCE=.\MissingEntryPoints.h
# End Source File
# Begin Source File

SOURCE=.\MudDetailsDlg.h
# End Source File
# Begin Source File

SOURCE=.\MudListDlg.h
# End Source File
# Begin Source File

SOURCE=.\MultiLineTriggerDlg.h
# End Source File
# Begin Source File

SOURCE=.\MUSHclient.h
# End Source File
# Begin Source File

SOURCE=.\mushview.h
# End Source File
# Begin Source File

SOURCE=.\mxp.h
# End Source File
# Begin Source File

SOURCE=.\MXPscriptRoutinesDlg.h
# End Source File
# Begin Source File

SOURCE=.\MySplitterWnd.h
# End Source File
# Begin Source File

SOURCE=.\MyStatusBar.h
# End Source File
# Begin Source File

SOURCE=.\OtherTypes.h
# End Source File
# Begin Source File

SOURCE=.\PaneChildWnd.h
# End Source File
# Begin Source File

SOURCE=.\PaneView.h
# End Source File
# Begin Source File

SOURCE=.\password.h
# End Source File
# Begin Source File

SOURCE=.\pcre_internal.h
# End Source File
# Begin Source File

SOURCE=.\PluginsDlg.h
# End Source File
# Begin Source File

SOURCE=.\PluginWizard.h
# End Source File
# Begin Source File

SOURCE=.\PluginWizardSheet.h
# End Source File
# Begin Source File

SOURCE=.\PrefsPropertyPages.h
# End Source File
# Begin Source File

SOURCE=.\PrefsPropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\ProgDlg.h
# End Source File
# Begin Source File

SOURCE=.\ProxyServerPasswordDlg.h
# End Source File
# Begin Source File

SOURCE=.\QuickConnectDlg.h
# End Source File
# Begin Source File

SOURCE=.\RecallDlg.h
# End Source File
# Begin Source File

SOURCE=.\RecallSearchDlg.h
# End Source File
# Begin Source File

SOURCE=.\RegexpProblemDlg.h
# End Source File
# Begin Source File

SOURCE=.\RegisterMUSHclient.h
# End Source File
# Begin Source File

SOURCE=.\RegistrationPropertyPage17.h
# End Source File
# Begin Source File

SOURCE=.\RegistrationPropertyPages.h
# End Source File
# Begin Source File

SOURCE=.\RegistrationPropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\ScriptErrorDlg.h
# End Source File
# Begin Source File

SOURCE=.\SendToAll.h
# End Source File
# Begin Source File

SOURCE=.\SendToAllDlg.h
# End Source File
# Begin Source File

SOURCE=.\sendvw.h
# End Source File
# Begin Source File

SOURCE=.\shs.h
# End Source File
# Begin Source File

SOURCE=.\SpellCheckDlg.h
# End Source File
# Begin Source File

SOURCE=.\Splash.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\TabDefaultsDlg.h
# End Source File
# Begin Source File

SOURCE=.\testcomm.h
# End Source File
# Begin Source File

SOURCE=.\testrslt.h
# End Source File
# Begin Source File

SOURCE=.\TextAttributesDlg.h
# End Source File
# Begin Source File

SOURCE=.\TextDocument.h
# End Source File
# Begin Source File

SOURCE=.\TextView.h
# End Source File
# Begin Source File

SOURCE=.\TimerDlg.h
# End Source File
# Begin Source File

SOURCE=.\TimerWnd.h
# End Source File
# Begin Source File

SOURCE=.\TipDlg.h
# End Source File
# Begin Source File

SOURCE=.\triggdlg.h
# End Source File
# Begin Source File

SOURCE=.\trigtest.h
# End Source File
# Begin Source File

SOURCE=.\UDPsocket.h
# End Source File
# Begin Source File

SOURCE=.\UniqueIDDlg.h
# End Source File
# Begin Source File

SOURCE=.\welcome.h
# End Source File
# Begin Source File

SOURCE=.\welcome1.h
# End Source File
# Begin Source File

SOURCE=.\winplace.h
# End Source File
# Begin Source File

SOURCE=.\WinsockInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\WorldConfiguration.h
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

SOURCE=.\CommandOptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\configuration.cpp
# End Source File
# Begin Source File

SOURCE=.\genpropertypage.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalPrefs.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalPrefsSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\PrefsPropertyPages.cpp
# End Source File
# Begin Source File

SOURCE=.\PrefsPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\scriptingoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\TreePropertySheet.cpp
# End Source File
# End Group
# Begin Group "Dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\aliasdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AsciiArtDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ButtonColour.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatCallDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatDetailsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatListDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatToAllDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChooseNotepadDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\cmdhist.cpp
# End Source File
# Begin Source File

SOURCE=.\ColourComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\ColourPickerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CompleteWordDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfirmPreamble.cpp
# End Source File
# Begin Source File

SOURCE=.\CreditsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugLuaDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugWorldInputDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\EditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\editmac.cpp
# End Source File
# Begin Source File

SOURCE=.\EditMultiLine.cpp
# End Source File
# Begin Source File

SOURCE=.\EditVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\EmoteToAllDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FindDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FunctionListDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GeneratedNameDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalChangeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GoToLineDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\HighlightPhraseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ImmediateDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ImportXMLdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\InsertUnicodeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyNameDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\logdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LuaChooseBox.cpp
# End Source File
# Begin Source File

SOURCE=.\LuaChooseList.cpp
# End Source File
# Begin Source File

SOURCE=.\LuaChooseListMulti.cpp
# End Source File
# Begin Source File

SOURCE=.\LuaGsubDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LuaInputBox.cpp
# End Source File
# Begin Source File

SOURCE=.\LuaInputEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MapCommentDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MapDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MapMoveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MissingEntryPoints.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiLineTriggerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MXPscriptRoutinesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\password.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProxyServerPasswordDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\QuickConnectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RecallDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RecallSearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RegexpProblemDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ScriptErrorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SendToAllDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SpellCheckDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TabDefaultsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\testcomm.cpp
# End Source File
# Begin Source File

SOURCE=.\testrslt.cpp
# End Source File
# Begin Source File

SOURCE=.\TextAttributesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TimerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TipDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\triggdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UniqueIDDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\welcome.cpp
# End Source File
# Begin Source File

SOURCE=.\welcome1.cpp
# End Source File
# Begin Source File

SOURCE=.\WinsockInfoDlg.cpp
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

SOURCE=.\PaneChildWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\sendvw.cpp
# End Source File
# Begin Source File

SOURCE=.\Splash.cpp
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

SOURCE=.\mxp.cpp
# End Source File
# Begin Source File

SOURCE=.\mxp_phases.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpClose.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpCloseAtomic.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpEnd.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpEntities.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpError.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpinit.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpMode.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpOnOff.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpOpenAtomic.cpp
# End Source File
# Begin Source File

SOURCE=.\mxpStart.cpp
# End Source File
# Begin Source File

SOURCE=.\mxputils.cpp
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\zlib\adler32.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\compress.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\crc32.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\deflate.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\gzio.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\inffast.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\inflate.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\inftrees.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\minigzip.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\trees.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\uncompr.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib\zutil.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /D "ZLIB_DLL" /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Group "XML"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xml_load_world.cpp
# End Source File
# Begin Source File

SOURCE=.\xml_save_world.cpp
# End Source File
# Begin Source File

SOURCE=.\xml_serialize.cpp
# End Source File
# Begin Source File

SOURCE=.\xmlparse.cpp
# End Source File
# End Group
# Begin Group "Plugins"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Plugins.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginWizard.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginWizardSheet.cpp
# End Source File
# End Group
# Begin Group "Panes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\paneline.cpp
# End Source File
# Begin Source File

SOURCE=.\PaneView.cpp
# End Source File
# End Group
# Begin Group "scripting"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bits.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\functionlist.cpp
# End Source File
# Begin Source File

SOURCE=.\lbc.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\lpeg.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\lrexlib.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\lua_compress.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\lua_methods.cpp

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lua_progressdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\lua_scripting.cpp
# End Source File
# Begin Source File

SOURCE=.\lua_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\md5.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\methods.cpp
# End Source File
# Begin Source File

SOURCE=.\mt19937ar.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\number.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\scriptengine.cpp
# End Source File
# Begin Source File

SOURCE=.\scripting.cpp
# End Source File
# Begin Source File

SOURCE=.\sha256.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "pcre"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcre_chartables.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_compile.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_exec.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_fullinfo.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_get.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_globals.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_maketables.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_newline.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_ord2utf8.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_study.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_tables.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_try_flipped.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_ucd.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_valid_utf8.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_version.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre_xclass.c

!IF  "$(CFG)" == "MUSHclient - Win32 Debug"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MUSHclient - Win32 Release"

# ADD CPP /D "HAVE_CONFIG_H"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Group "PNG"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\png.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngerror.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pnggccrd.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngget.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngmem.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngpread.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngread.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngrio.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngrtran.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngrutil.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngset.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngtrans.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngvcrd.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngwio.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngwrite.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngwtran.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\pngwutil.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Sqlite"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\lsqlite3.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\sqlite3.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Source File

SOURCE=.\manifest.xml
# End Source File
# Begin Source File

SOURCE=.\install\readme.txt
# End Source File
# End Target
# End Project
# Section MUSHclient : {00000000-0001-0000-0000-000000000000}
# 	1:23:CG_IDR_POPUP_MAIN_FRAME:126
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
# Section MUSHclient : {00000000-0000-0000-0000-000000000000}
# 	1:22:CG_IDR_POPUP_SEND_VIEW:124
# End Section
# Section MUSHclient : {52A03A14-D2CC-0056-A8D2-560020D55600}
# 	1:21:CG_IDR_POPUP_MUSHVIEW:125
# End Section
# Section MUSHclient : {00000000-000C-0000-0C00-00004944445F}
# 	1:26:CG_IDR_POPUP_ACTIVITY_VIEW:127
# End Section
