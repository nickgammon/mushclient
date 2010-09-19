#include "stdafx.h"
#include "..\mainfrm.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "..\dialogs\FunctionListDlg.h"
#include "..\dialogs\CompleteWordDlg.h"

#define FUNCTIONS_FIRST_MENU 10000
#define FUNCTIONS_MENU_COUNT 30   // number of functions menu items we support

extern set<string> LuaFunctionsSet;

// select concat("'", name, "', ") FROM functions ORDER BY name; 
// then change ' to "

tInternalFunctionsTable InternalFunctionsTable [] = {

// FUNCTION                     ARGUMENTS

{ "Accelerator" ,                "( Key , Send )" } ,
{ "AcceleratorList" ,            "( )" } ,
{ "AcceleratorTo" ,              "( Key , Send , SendTo )" } ,
{ "Activate" ,                   "( )" } ,
{ "ActivateClient" ,             "( )" } ,
{ "ActivateNotepad" ,            "( Title )" } ,
{ "AddAlias" ,                   "( AliasName , MatchText , ResponseText , Flags , ScriptName )" } ,
{ "AddFont" ,                    "( PathName )" } ,
{ "AddMapperComment" ,           "( Comment )" } ,
{ "AddSpellCheckWord" ,          "( OriginalWord , ActionCode , ReplacementWord )" } ,
{ "AddTimer" ,                   "( TimerName , Hour , Minute , Second , ResponseText , Flags , ScriptName )" } ,
{ "AddToMapper" ,                "( Direction , Reverse )" } ,
{ "AddTrigger" ,                 "( TriggerName , MatchText , ResponseText , Flags , Colour , Wildcard , SoundFileName , ScriptName )" } ,
{ "AddTriggerEx" ,               "( TriggerName , MatchText , ResponseText , Flags , Colour , Wildcard , SoundFileName , ScriptName , SendTo , Sequence )" } ,
{ "AdjustColour" ,               "( Colour , Method )" } ,
{ "ANSI" ,                       "( Code )" } ,
{ "AnsiNote" ,                   "( Text )" } ,
{ "AppendToNotepad" ,            "( Title , Contents )" } ,
{ "ArrayClear" ,                 "( Name )" } ,
{ "ArrayCount" ,                 "( )" } ,
{ "ArrayCreate" ,                "( Name )" } ,
{ "ArrayDelete" ,                "( Name )" } ,
{ "ArrayDeleteKey" ,             "( Name , Key )" } ,
{ "ArrayExists" ,                "( Name )" } ,
{ "ArrayExport" ,                "( Name , Delimiter )" } ,
{ "ArrayExportKeys" ,            "( Name , Delimiter )" } ,
{ "ArrayGet" ,                   "( Name , Key )" } ,
{ "ArrayGetFirstKey" ,           "( Name )" } ,
{ "ArrayGetLastKey" ,            "( Name )" } ,
{ "ArrayImport" ,                "( Name , Values , Delimiter )" } ,
{ "ArrayKeyExists" ,             "( Name , Key )" } ,
{ "ArrayListAll" ,               "( )" } ,
{ "ArrayListKeys" ,              "( Name )" } ,
{ "ArrayListValues" ,            "( Name )" } ,
{ "ArraySet" ,                   "( Name , Key , Value )" } ,
{ "ArraySize" ,                  "( Name )" } ,
{ "Base64Decode" ,               "( Text )" } ,
{ "Base64Encode" ,               "( Text , MultiLine )" } ,
{ "BlendPixel" ,                 "( Blend , Base , Mode , Opacity )" } ,
{ "BoldColour" ,                 "( WhichColour , NewValue )" } ,
{ "BroadcastPlugin" ,            "( Message , Text )" } ,
{ "CallPlugin" ,                 "( PluginID , Routine , Argument )" } ,
{ "ChangeDir" ,                  "( Path )" } ,
{ "ChatAcceptCalls" ,            "( Port )" } ,
{ "ChatCall" ,                   "( Server , Port )" } ,
{ "ChatCallzChat" ,              "( Server , Port )" } ,
{ "ChatDisconnect" ,             "( ID )" } ,
{ "ChatDisconnectAll" ,          "( )" } ,
{ "ChatEverybody" ,              "( Message , Emote )" } ,
{ "ChatGetID" ,                  "( Who )" } ,
{ "ChatGroup" ,                  "( Group , Message , Emote )" } ,
{ "ChatID" ,                     "( ID , Message , Emote )" } ,
{ "ChatMessage" ,                "( ID , Message , Text )" } ,
{ "ChatNameChange" ,             "( NewName )" } ,
{ "ChatNote" ,                   "( NoteType , Message )" } ,
{ "ChatPasteEverybody" ,         "( )" } ,
{ "ChatPasteText" ,              "( ID )" } ,
{ "ChatPeekConnections" ,        "( ID )" } ,
{ "ChatPersonal" ,               "( Who , Message , Emote )" } ,
{ "ChatPing" ,                   "( ID )" } ,
{ "ChatRequestConnections" ,     "( ID )" } ,
{ "ChatSendFile" ,               "( ID , FileName )" } ,
{ "ChatStopAcceptingCalls" ,     "( )" } ,
{ "ChatStopFileTransfer" ,       "( ID )" } ,
{ "CloseLog" ,                   "( )" } ,
{ "CloseNotepad" ,               "( Title , QuerySave )" } ,
{ "ColourNameToRGB" ,            "( Name )" } ,
{ "ColourNote" ,                 "( TextColour , BackgroundColour , Text )" } ,
{ "ColourTell" ,                 "( TextColour , BackgroundColour , Text )" } ,
{ "Connect" ,                    "( )" } ,
{ "CreateGUID" ,                 "( )" } ,
{ "CustomColourBackground" ,     "( WhichColour , NewValue )" } ,
{ "CustomColourText" ,           "( WhichColour , NewValue )" } ,
{ "DatabaseChanges" ,            "( DbName )" } ,
{ "DatabaseClose" ,              "( DbName )" } ,
{ "DatabaseColumnName" ,         "( DbName , Column )" } ,
{ "DatabaseColumnNames" ,        "( DbName )" } ,
{ "DatabaseColumns" ,            "( DbName )" } ,
{ "DatabaseColumnText" ,         "( DbName , Column )" } ,
{ "DatabaseColumnType" ,         "( DbName , Column )" } ,
{ "DatabaseColumnValue" ,        "( DbName , Column )" } ,
{ "DatabaseColumnValues" ,       "( DbName )" } ,
{ "DatabaseError" ,              "( DbName )" } ,
{ "DatabaseExec" ,               "( DbName , Sql )" } ,
{ "DatabaseFinalize" ,           "( DbName )" } ,
{ "DatabaseInfo" ,               "( DbName , InfoType )" } ,
{ "DatabaseLastInsertRowid" ,    "( DbName )" } ,
{ "DatabaseList" ,               "( )" } ,
{ "DatabaseOpen" ,               "( DbName , Filename , Flags )" } ,
{ "DatabasePrepare" ,            "( DbName , Sql )" } ,
{ "DatabaseReset" ,              "( DbName )" } ,
{ "DatabaseStep" ,               "( DbName )" } ,
{ "DatabaseTotalChanges" ,       "( DbName )" } ,
{ "Debug" ,                      "( Command )" } ,
{ "DeleteAlias" ,                "( AliasName )" } ,
{ "DeleteAliasGroup" ,           "( GroupName )" } ,
{ "DeleteAllMapItems" ,          "( )" } ,
{ "DeleteCommandHistory" ,       "( )" } ,
{ "DeleteGroup" ,                "( GroupName )" } ,
{ "DeleteLastMapItem" ,          "( )" } ,
{ "DeleteLines" ,                "( Count )" } ,
{ "DeleteOutput" ,               "( )" } ,
{ "DeleteTemporaryAliases" ,     "( )" } ,
{ "DeleteTemporaryTimers" ,      "( )" } ,
{ "DeleteTemporaryTriggers" ,    "( )" } ,
{ "DeleteTimer" ,                "( TimerName )" } ,
{ "DeleteTimerGroup" ,           "( GroupName )" } ,
{ "DeleteTrigger" ,              "( TriggerName )" } ,
{ "DeleteTriggerGroup" ,         "( GroupName )" } ,
{ "DeleteVariable" ,             "( VariableName )" } ,
{ "DiscardQueue" ,               "( )" } ,
{ "Disconnect" ,                 "( )" } ,
{ "DoAfter" ,                    "( Seconds , SendText )" } ,
{ "DoAfterNote" ,                "( Seconds , NoteText )" } ,
{ "DoAfterSpecial" ,             "( Seconds , SendText , SendTo )" } ,
{ "DoAfterSpeedWalk" ,           "( Seconds , SendText )" } ,
{ "DoCommand" ,                  "( Command )" } ,
{ "EchoInput" ,                  "( )" } ,
{ "EditDistance" ,               "( Source , Target )" } ,
{ "EnableAlias" ,                "( AliasName , Enabled )" } ,
{ "EnableAliasGroup" ,           "( GroupName , Enabled )" } ,
{ "EnableGroup" ,                "( GroupName , Enabled )" } ,
{ "EnableMapping" ,              "( Enabled )" } ,
{ "EnablePlugin" ,               "( PluginID , Enabled )" } ,
{ "EnableTimer" ,                "( TimerName , Enabled )" } ,
{ "EnableTimerGroup" ,           "( GroupName , Enabled )" } ,
{ "EnableTrigger" ,              "( TriggerName , Enabled )" } ,
{ "EnableTriggerGroup" ,         "( GroupName , Enabled )" } ,
{ "ErrorDesc" ,                  "( Code )" } ,
{ "EvaluateSpeedwalk" ,          "( SpeedWalkString )" } ,
{ "Execute" ,                    "( Command )" } ,
{ "ExportXML" ,                  "( Type , Name )" } ,
{ "FilterPixel" ,                "( Pixel , Operation , Options )" } ,
{ "FixupEscapeSequences" ,       "( Source )" } ,
{ "FixupHTML" ,                  "( StringToConvert )" } ,
{ "FlashIcon" ,                  "( )" } ,
{ "FlushLog" ,                   "( )" } ,
{ "GenerateName" ,               "( )" } ,
{ "GetAlias" ,                   "( AliasName , MatchText , ResponseText , Parameter , Flags , ScriptName )" } ,
{ "GetAliasInfo" ,               "( AliasName , InfoType )" } ,
{ "GetAliasList" ,               "( )" } ,
{ "GetAliasOption" ,             "( AliasName , OptionName )" } ,
{ "GetAliasWildcard" ,           "( AliasName , WildcardName )" } ,
{ "GetAlphaOption" ,             "( OptionName )" } ,
{ "GetAlphaOptionList" ,         "( )" } ,
{ "GetChatInfo" ,                "( ChatID , InfoType )" } ,
{ "GetChatList" ,                "( )" } ,
{ "GetChatOption" ,              "( ID , OptionName )" } ,
{ "GetClipboard" ,               "( )" } ,
{ "GetCommand" ,                 "( )" } ,
{ "GetCommandList" ,             "( Count )" } ,
{ "GetConnectDuration" ,         "( )" } ,
{ "GetCurrentValue" ,            "( OptionName )" } ,
{ "GetCustomColourName" ,        "( WhichColour )" } ,
{ "GetDefaultValue" ,            "( OptionName )" } ,
{ "GetDeviceCaps" ,              "( Index )" } ,
{ "GetEntity" ,                  "( Name )" } ,
{ "GetFrame" ,                   "( )" } ,
{ "GetGlobalOption" ,            "( OptionName )" } ,
{ "GetGlobalOptionList" ,        "( )" } ,
{ "GetHostAddress" ,             "( HostName )" } ,
{ "GetHostName" ,                "( IPaddress )" } ,
{ "GetInfo" ,                    "( InfoType )" } ,
{ "GetInternalCommandsList" ,    "( )" } ,
{ "GetLineCount" ,               "( )" } ,
{ "GetLineInfo" ,                "( LineNumber , InfoType )" } ,
{ "GetLinesInBufferCount" ,      "( )" } ,
{ "GetLoadedValue" ,             "( OptionName )" } ,
{ "GetMainWindowPosition" ,      "( )" } ,
{ "GetMapColour" ,               "( Which )" } ,
{ "GetMappingCount" ,            "( )" } ,
{ "GetMappingItem" ,             "( Item )" } ,
{ "GetMappingString" ,           "( )" } ,
{ "GetNotepadLength" ,           "( Title )" } ,
{ "GetNotepadList" ,             "( All )" } ,
{ "GetNotepadText" ,             "( Title )" } ,
{ "GetNotepadWindowPosition" ,   "( Title )" } ,
{ "GetNotes" ,                   "( )" } ,
{ "GetNoteStyle" ,               "( )" } ,
{ "GetOption" ,                  "( OptionName )" } ,
{ "GetOptionList" ,              "( )" } ,
{ "GetPluginAliasInfo" ,         "( PluginID , AliasName , InfoType )" } ,
{ "GetPluginAliasList" ,         "( PluginID )" } ,
{ "GetPluginAliasOption" ,       "( PluginID , AliasName , OptionName )" } ,
{ "GetPluginID" ,                "( )" } ,
{ "GetPluginInfo" ,              "( PluginID , InfoType )" } ,
{ "GetPluginList" ,              "( )" } ,
{ "GetPluginName" ,              "( )" } ,
{ "GetPluginTimerInfo" ,         "( PluginID , TimerName , InfoType )" } ,
{ "GetPluginTimerList" ,         "( PluginID )" } ,
{ "GetPluginTimerOption" ,       "( PluginID , TimerName , OptionName )" } ,
{ "GetPluginTriggerInfo" ,       "( PluginID , TriggerName , InfoType )" } ,
{ "GetPluginTriggerList" ,       "( PluginID )" } ,
{ "GetPluginTriggerOption" ,     "( PluginID , TriggerName , OptionName )" } ,
{ "GetPluginVariable" ,          "( PluginID , VariableName )" } ,
{ "GetPluginVariableList" ,      "( PluginID )" } ,
{ "GetQueue" ,                   "( )" } ,
{ "GetReceivedBytes" ,           "( )" } ,
{ "GetRecentLines" ,             "( Count )" } ,
{ "GetScriptTime" ,              "( )" } ,
{ "GetSelectionEndColumn" ,      "( )" } ,
{ "GetSelectionEndLine" ,        "( )" } ,
{ "GetSelectionStartColumn" ,    "( )" } ,
{ "GetSelectionStartLine" ,      "( )" } ,
{ "GetSentBytes" ,               "( )" } ,
{ "GetSoundStatus" ,             "( Buffer )" } ,
{ "GetStyleInfo" ,               "( LineNumber , StyleNumber , InfoType )" } ,
{ "GetSysColor" ,                "( Index )" } ,
{ "GetSystemMetrics" ,           "( Index )" } ,
{ "GetTimer" ,                   "( TimerName , Hour , Minute , Second , ResponseText , Flags , ScriptName )" } ,
{ "GetTimerInfo" ,               "( TimerName , InfoType )" } ,
{ "GetTimerList" ,               "( )" } ,
{ "GetTimerOption" ,             "( TimerName , OptionName )" } ,
{ "GetTrigger" ,                 "( TriggerName , MatchText , ResponseText , Flags , Colour , Wildcard , SoundFileName , ScriptName )" } ,
{ "GetTriggerInfo" ,             "( TriggerName , InfoType )" } ,
{ "GetTriggerList" ,             "( )" } ,
{ "GetTriggerOption" ,           "( TriggerName , OptionName )" } ,
{ "GetTriggerWildcard" ,         "( TriggerName , WildcardName )" } ,
{ "GetUdpPort" ,                 "( First , Last )" } ,
{ "GetUniqueID" ,                "( )" } ,
{ "GetUniqueNumber" ,            "( )" } ,
{ "GetVariable" ,                "( VariableName )" } ,
{ "GetVariableList" ,            "( )" } ,
{ "GetWorld" ,                   "( WorldName )" } ,
{ "GetWorldById" ,               "( WorldID )" } ,
{ "GetWorldID" ,                 "( )" } ,
{ "GetWorldIdList" ,             "( )" } ,
{ "GetWorldList" ,               "( )" } ,
{ "GetWorldWindowPosition" ,     "( )" } ,
{ "GetWorldWindowPositionX" ,    "( Which )" } ,
{ "GetXMLEntity" ,               "( Entity )" } ,
{ "Hash" ,                       "( Text )" } ,
{ "Help" ,                       "( Name )" } ,
{ "Hyperlink" ,                  "( Action , Text , Hint , TextColour , BackColour , URL )" } ,
{ "ImportXML" ,                  "( XML )" } ,
{ "Info" ,                       "( Message )" } ,
{ "InfoBackground" ,             "( Name )" } ,
{ "InfoClear" ,                  "( )" } ,
{ "InfoColour" ,                 "( Name )" } ,
{ "InfoFont" ,                   "( FontName , Size , Style )" } ,
{ "IsAlias" ,                    "( AliasName )" } ,
{ "IsConnected" ,                "( )" } ,
{ "IsLogOpen" ,                  "( )" } ,
{ "IsPluginInstalled" ,          "( PluginID )" } ,
{ "IsTimer" ,                    "( TimerName )" } ,
{ "IsTrigger" ,                  "( TriggerName )" } ,
{ "LoadPlugin" ,                 "( FileName )" } ,
{ "LogInput" ,                   "( )" } ,
{ "LogNotes" ,                   "( )" } ,
{ "LogOutput" ,                  "( )" } ,
{ "LogSend" ,                    "( Message )" } ,
{ "MakeRegularExpression" ,      "( Text )" } ,
{ "MapColour" ,                  "( Original , Replacement )" } ,
{ "MapColourList" ,              "( )" } ,
{ "Mapping" ,                    "( )" } ,
{ "Metaphone" ,                  "( Word , Length )" } ,
{ "MoveMainWindow" ,             "( Left , Top , Width , Height )" } ,
{ "MoveNotepadWindow" ,          "( Title , Left , Top , Width , Height )" } ,
{ "MoveWorldWindow" ,            "( Left , Top , Width , Height )" } ,
{ "MoveWorldWindowX" ,           "( Left , Top , Width , Height , Which )" } ,
{ "MtRand" ,                     "( )" } ,
{ "MtSrand" ,                    "( Seed )" } ,
{ "NormalColour" ,               "( WhichColour , NewValue )" } ,
{ "Note" ,                       "( Message )" } ,
{ "NoteColour" ,                 "( )" } ,
{ "NoteColourBack" ,             "( )" } ,
{ "NoteColourFore" ,             "( )" } ,
{ "NoteColourName" ,             "( Foreground , Background )" } ,
{ "NoteColourRGB" ,              "( Foreground , Background )" } ,
{ "NoteHr" ,                     "( )" } ,
{ "NotepadColour" ,              "( Title , TextColour , BackgroundColour )" } ,
{ "NotepadFont" ,                "( Title , FontName , Size , Style , Charset )" } ,
{ "NotepadReadOnly" ,            "( Title , ReadOnly )" } ,
{ "NotepadSaveMethod" ,          "( Title , Method )" } ,
{ "NoteStyle" ,                  "( Style )" } ,
{ "Open" ,                       "( FileName )" } ,
{ "OpenBrowser" ,                "( URL )" } ,
{ "OpenLog" ,                    "( LogFileName , Append )" } ,
{ "PasteCommand" ,               "( Text )" } ,
{ "Pause" ,                      "( Flag )" } ,
{ "PickColour" ,                 "( Suggested )" } ,
{ "PlaySound" ,                  "( Buffer , FileName , Loop , Volume , Pan )" } ,
{ "PluginSupports" ,             "( PluginID , Routine )" } ,
{ "PushCommand" ,                "( )" } ,
{ "Queue" ,                      "( Message , Echo )" } ,
{ "ReadNamesFile" ,              "( FileName )" } ,
{ "Redraw" ,                     "( )" } ,
{ "ReloadPlugin" ,               "( PluginID )" } ,
{ "RemoveBacktracks" ,           "( Path )" } ,
{ "RemoveMapReverses" ,          "( )" } ,
{ "Repaint" ,                    "( )" } ,
{ "Replace" ,                    "( Source , SearchFor , ReplaceWith , Multiple )" } ,
{ "ReplaceNotepad" ,             "( Title , Contents )" } ,
{ "Reset" ,                      "( )" } ,
{ "ResetIP" ,                    "( )" } ,
{ "ResetStatusTime" ,            "( )" } ,
{ "ResetTimer" ,                 "( TimerName )" } ,
{ "ResetTimers" ,                "( )" } ,
{ "ReverseSpeedwalk" ,           "( SpeedWalkString )" } ,
{ "RGBColourToName" ,            "( Colour )" } ,
{ "Save" ,                       "( Name )" } ,
{ "SaveNotepad" ,                "( Title , FileName , ReplaceExisting )" } ,
{ "SaveState" ,                  "( )" } ,
{ "SelectCommand" ,              "( )" } ,
{ "Send" ,                       "( Message )" } ,
{ "SendImmediate" ,              "( Message )" } ,
{ "SendNoEcho" ,                 "( Message )" } ,
{ "SendPkt" ,                    "( Packet )" } ,
{ "SendPush" ,                   "( Message )" } ,
{ "SendSpecial" ,                "( Message , Echo , Queue , Log , History )" } ,
{ "SendToNotepad" ,              "( Title , Contents )" } ,
{ "SetAliasOption" ,             "( AliasName , OptionName , Value )" } ,
{ "SetAlphaOption" ,             "( OptionName , Value )" } ,
{ "SetBackgroundColour" ,        "( Colour )" } ,
{ "SetBackgroundImage" ,         "( FileName , Mode )" } ,
{ "SetChanged" ,                 "( ChangedFlag )" } ,
{ "SetChatOption" ,              "( ID , OptionName , Value )" } ,
{ "SetClipboard" ,               "( Text )" } ,
{ "SetCommand" ,                 "( Message )" } ,
{ "SetCommandSelection" ,        "( First , Last )" } ,
{ "SetCommandWindowHeight" ,     "( Height )" } ,
{ "SetCursor" ,                  "( Cursor )" } ,
{ "SetCustomColourName" ,        "( WhichColour , Name )" } ,
{ "SetEntity" ,                  "( Name , Contents )" } ,
{ "SetForegroundImage" ,         "( FileName , Mode )" } ,
{ "SetInputFont" ,               "( FontName , PointSize , Weight , Italic )" } ,
{ "SetNotes" ,                   "( Message )" } ,
{ "SetOption" ,                  "( OptionName , Value )" } ,
{ "SetOutputFont" ,              "( FontName , PointSize )" } ,
{ "SetScroll",                   "( Position ,  Visible )" }, 
{ "SetStatus" ,                  "( Message )" } ,
{ "SetTimerOption" ,             "( TimerName , OptionName , Value )" } ,
{ "SetToolBarPosition" ,         "( Which , Float , Side , Top , Left )" } ,
{ "SetTriggerOption" ,           "( TriggerName , OptionName , Value )" } ,
{ "SetVariable" ,                "( VariableName , Contents )" } ,
{ "SetWorldWindowStatus" ,       "( Parameter )" } ,
{ "ShiftTabCompleteItem" ,       "( Item )" } ,
{ "ShowInfoBar" ,                "( Visible )" } ,
{ "Simulate" ,                   "( Text )" } ,
{ "Sound" ,                      "( SoundFileName )" } ,
{ "SpeedWalkDelay" ,             "( )" } ,
{ "SpellCheck" ,                 "( Text )" } ,
{ "SpellCheckCommand" ,          "( StartCol , EndCol )" } ,
{ "SpellCheckDlg" ,              "( Text )" } ,
{ "StopSound" ,                  "( Buffer )" } ,
{ "StripANSI" ,                  "( Message )" } ,
{ "Tell" ,                       "( Message )" } ,
{ "TextRectangle" ,              "( Left , Top , Right , Bottom , BorderOffset , BorderColour , BorderWidth , OutsideFillColour , OutsideFillStyle )" } ,
{ "Trace" ,                      "( )" } ,
{ "TraceOut" ,                   "( Message )" } ,
{ "TranslateDebug" ,             "( Message )" } ,
{ "TranslateGerman" ,            "( Text )" } ,
{ "Transparency" ,               "( Key , Amount )" } ,
{ "Trim" ,                       "( Source )" } ,
{ "UdpListen" ,                  "( IP , Port , Script )" } ,
{ "UdpPortList" ,                "( )" } ,
{ "UdpSend" ,                    "( IP , Port , Text )" } ,
{ "Version" ,                    "( )" } ,
{ "WindowAddHotspot" ,           "( WindowName , HotspotId , Left , Top , Right , Bottom , MouseOver , CancelMouseOver , MouseDown , CancelMouseDown , MouseUp , TooltipText , Cursor , Flags )" } ,
{ "WindowArc" ,                  "( WindowName , Left , Top , Right , Bottom , x1 , y1 , x2 , y2 , PenColour , PenStyle , PenWidth )" } ,
{ "WindowBezier" ,               "( WindowName , Points , PenColour , PenStyle , PenWidth )" } ,
{ "WindowBlendImage" ,           "( WindowName , ImageId , Left , Top , Right , Bottom , Mode , Opacity , SrcLeft , SrcTop , SrcRight , SrcBottom )" } ,
{ "WindowCircleOp" ,             "( WindowName , Action , Left , Top , Right , Bottom , PenColour , PenStyle , PenWidth , BrushColour , BrushStyle , Extra1 , Extra2 , Extra3 , Extra4 )" } ,
{ "WindowCreate" ,               "( WindowName , Left , Top , Width , Height , Position , Flags , BackgroundColour )" } ,
{ "WindowCreateImage" ,          "( WindowName , ImageId , Row8 , Row7 , Row6 , Row5 , Row4 , Row3 , Row2 , Row1 )" } ,
{ "WindowDelete" ,               "( WindowName )" } ,
{ "WindowDeleteAllHotspots" ,    "( WindowName )" } ,
{ "WindowDeleteHotspot" ,        "( WindowName , HotspotId )" } ,
{ "WindowDragHandler" ,          "( WindowName , HotspotId , MoveCallback , ReleaseCallback , Flags )" } ,
{ "WindowDrawImage" ,            "( WindowName , ImageId , Left , Top , Right , Bottom , Mode , SrcLeft , SrcTop , SrcRight , SrcBottom )" } ,
{ "WindowDrawImageAlpha" ,       "( WindowName , ImageId , Left , Top , Right , Bottom , Opacity , SrcLeft , SrcTop )" } ,
{ "WindowFilter" ,               "( WindowName , Left , Top , Right , Bottom , Operation , Options )" } ,
{ "WindowFont" ,                 "( WindowName , FontId , FontName , Size , Bold , Italic , Underline , Strikeout , Charset , PitchAndFamily )" } ,
{ "WindowFontInfo" ,             "( WindowName , FontId , InfoType )" } ,
{ "WindowFontList" ,             "( WindowName )" } ,
{ "WindowGetImageAlpha" ,        "( WindowName , ImageId , Left , Top , Right , Bottom , SrcLeft , SrcTop )" } ,
{ "WindowGetPixel" ,             "( WindowName , x , y )" } ,
{ "WindowGradient" ,             "( WindowName , Left , Top , Right , Bottom , StartColour , EndColour , Mode )" } ,
{ "WindowHotspotInfo" ,          "( WindowName , HotspotId , InfoType )" } ,
{ "WindowHotspotList" ,          "( WindowName )" } ,
{ "WindowHotspotTooltip" ,       "( WindowName , HotspotId , TooltipText )" } ,
{ "WindowImageFromWindow" ,      "( WindowName , ImageId , SourceWindow )" } ,
{ "WindowImageInfo" ,            "( WindowName , ImageId , InfoType )" } ,
{ "WindowImageList" ,            "( WindowName )" } ,
{ "WindowImageOp" ,              "( WindowName , Action , Left , Top , Right , Bottom , PenColour , PenStyle , PenWidth , BrushColour , ImageId , EllipseWidth , EllipseHeight )" } ,
{ "WindowInfo" ,                 "( WindowName , InfoType )" } ,
{ "WindowLine" ,                 "( WindowName , x1 , y1 , x2 , y2 , PenColour , PenStyle , PenWidth )" } ,
{ "WindowList" ,                 "( )" } ,
{ "WindowLoadImage" ,            "( WindowName , ImageId , FileName )" } ,
{ "WindowMenu" ,                 "( WindowName , Left , Top , Items )" } ,
{ "WindowMergeImageAlpha" ,      "( WindowName , ImageId , MaskId , Left , Top , Right , Bottom , Mode , Opacity , SrcLeft , SrcTop , SrcRight , SrcBottom )" } ,
{ "WindowMoveHotspot" ,          "( WindowName , HotspotId , Left , Top , Right , Bottom )" },
{ "WindowPolygon" ,              "( WindowName , Points , PenColour , PenStyle , PenWidth , BrushColour , BrushStyle , Close , Winding )" } ,
{ "WindowPosition" ,             "( WindowName , Left , Top , Position , Flags )" } ,
{ "WindowResize",                "( WindowName , Width , Height , BackgroundColour )" }, 
{ "WindowRectOp" ,               "( WindowName , Action , Left , Top , Right , Bottom , Colour1 , Colour2 )" } ,
{ "WindowScrollwheelHandler" ,   "( WindowName , HotspotId , MoveCallback )" } ,
{ "WindowSetPixel" ,             "( WindowName , x , y , Colour )" } ,
{ "WindowShow" ,                 "( WindowName , Show )" } ,
{ "WindowText" ,                 "( WindowName , FontId , Text , Left , Top , Right , Bottom , Colour , Unicode )" } ,
{ "WindowTextWidth" ,            "( WindowName , FontId , Text , Unicode )" } ,
{ "WindowTransformImage" ,       "( WindowName , ImageId , Left , Top , Mode , Mxx , Mxy , Myx , Myy ) " },
{ "WindowWrite" ,                "( WindowName , FileName )" } ,                          
{ "WorldAddress" ,               "( )" } ,
{ "WorldName" ,                  "( )" } ,
{ "WorldPort" ,                  "( )" } ,
{ "WriteLog" ,                   "( Message )" } ,
 
{  "", "" }    // end of table marker

  };

CString GetSelectedFunction (CString & strWindowContents, int & nStartChar, int & nEndChar)
  {

  CString strWord;

  if (!strWindowContents.IsEmpty ())
    {
    // if no selection try to get word under cursor


    if (nStartChar == nEndChar)
      {
      nStartChar--;   // go back one so we are not at end of string

      // scan backwards from cursor
      while (nStartChar >= 0 && 
             strWord.GetLength () < 30 &&
             (isalnum (strWindowContents [nStartChar]) || 
              strWindowContents [nStartChar] == '.' || 
              strWindowContents [nStartChar] == '-' || 
              strWindowContents [nStartChar] == '_')
             )
        {
        strWord = strWindowContents.Mid (nStartChar, 1) + strWord;
        nStartChar--;
        }

      nStartChar++;

      // scan forwards from cursor
      while (nEndChar < strWindowContents.GetLength () && 
             strWord.GetLength () < 30 &&
             (isalnum (strWindowContents [nEndChar]) || 
              strWindowContents [nEndChar] == '.' || 
              strWindowContents [nEndChar] == '-' || 
              strWindowContents [nEndChar] == '_')
             )
        {
        strWord = strWord + strWindowContents.Mid (nEndChar, 1);
        nEndChar++;
        }

      strWord.MakeLower ();
      strWord.Replace ("world.", "");  // if they said world.note, just take note    

      } // end of no selection
    else
      {
      if (nEndChar > nStartChar &&
          (nEndChar - nStartChar) < 30)
        {

        // skip spaces, changing the selection while we do it (for replacement purposes)
        while (isspace (strWindowContents [nStartChar]) &&
               nEndChar > nStartChar)
          nStartChar++;

        // ditto for trailing spaces
        while (isspace (strWindowContents [nEndChar - 1]) &&
               (nEndChar - 1) > nStartChar)
          nEndChar--;

        strWindowContents = strWindowContents.Mid (nStartChar, nEndChar - nStartChar);
        strWindowContents.MakeLower ();
        strWindowContents.Replace ("@world.", ""); // ditto for @world.Note  (Ruby)
        strWindowContents.Replace ("world.", "");  // if they said world.note, just take note    
        strWindowContents.Replace ("$world->", ""); // ditto for $world->note  (Perl)
        bool bFunction = true;
        for (int i = 0; i < strWindowContents.GetLength (); i++)
          if (!isalnum (strWindowContents [i]))
            bFunction = false;
        if (bFunction)
          strWord = strWindowContents;
        }

      } // we have a selection

     }   // end of some selection supplied

  return strWord;

  } // end of GetSelectedFunction

void ShowFunctionslist (CString & strWindowContents, int nStartChar, int nEndChar, const bool bLua)
{

  // skip backwards over any whitespace or LH brackets (which the new
  // function context stuff probably put there)

  if (!strWindowContents.IsEmpty ())
    {

    if (nStartChar == nEndChar)
      {
      nStartChar--;    // error if at end of string, so go back one
      nEndChar--;

      // scan backwards from cursor
      while (nStartChar >= 0 && 
             (isspace (strWindowContents [nStartChar]) || 
              strWindowContents [nStartChar] == '(')
             )
        {
        nStartChar--;
        nEndChar--;
        }

      } // no selection
      nStartChar++;    // compensate for earlier subtract
      nEndChar++;

     }   // end of some contents


CString strWord = GetSelectedFunction (strWindowContents, nStartChar, nEndChar);

CFunctionListDlg dlg;

  dlg.m_bLua = bLua;
  dlg.m_strFilter = strWord;     // selected word from dialog/text window

  if (dlg.DoModal () == IDCANCEL || dlg.m_strResult.IsEmpty ())
    return;

  ShowHelp ("", dlg.m_strResult); // already has prefix

}


void CMainFrame::OnGameFunctionslist() 
  {
  CString s;
  ShowFunctionslist (s, true);
  }

void ShowHelp (const CString strPrefix, const CString strTopic)
  {
CWinApp* pApp = AfxGetApp();
ASSERT_VALID(pApp);
ASSERT(pApp->m_pszHelpFilePath != NULL);

  CString strFull = strPrefix;    // eg. FNC_
  strFull += strTopic;            // eg. Note

// WinHelp will treat backslashes as special, so double them
string sPath = FindAndReplace (pApp->m_pszHelpFilePath, "\\", "\\\\");

CString strCommand;

  strCommand.Format ("JI(\"%s\",\"%s\")",   // jump to help topic by ID (JumpID)
                   sPath.c_str (),          // path of help file
                   strFull);                // topic, eg. FNC_Note

  // show the help
  ::WinHelp (Frame, pApp->m_pszHelpFilePath, HELP_COMMAND, (DWORD) (LPCTSTR) strCommand);

  } // end of ShowHelp 


void FunctionMenu (CEdit & editctrl, const bool bLua, set<string> * extraItems, const bool bFunctions)
  {

  int nStartChar,
      nEndChar;
  CString strWindowContents;

  // find the selection range
  editctrl.GetSel(nStartChar, nEndChar);

  if (nEndChar < 0)
    nEndChar = nStartChar;

  // get window text
  editctrl.GetWindowText (strWindowContents);

  CString strWord = GetSelectedFunction (strWindowContents, nStartChar, nEndChar);

  if (strWord.IsEmpty ())
    return;

  CCompleteWordDlg dlg;

  dlg.m_bLua = bLua;
  dlg.m_extraItems = extraItems;
  dlg.m_bFunctions = bFunctions;
  dlg.m_pt = editctrl.PosFromChar (nEndChar - 1);  // strangely doesn't work at end of line

  dlg.m_pt.x += 10;  // small gap
  dlg.m_pt.y += 10;  // small adjustment lalala

  editctrl.ClientToScreen(&dlg.m_pt);

  dlg.m_strFilter = strWord;     // selected word from dialog/text window

  if (dlg.DoModal () == IDCANCEL || dlg.m_strResult.IsEmpty ())
    return;

  // adjust selection in case we wandered around catching characters around the cursor

  editctrl.SetSel (nStartChar, nEndChar);

  // replace selection with the word they chose
  editctrl.ReplaceSel (dlg.m_strResult, TRUE);
              
  // New in version 4.57 - add the argument list after the function name

  // find the new selection range (ie. where the cursor is after the function name)
  editctrl.GetSel(nStartChar, nEndChar);
  if (nEndChar < 0)
    nEndChar = nStartChar;

  // get window text with function name in it
  editctrl.GetWindowText (strWindowContents);

  nStartChar = nEndChar;
  bool bFoundBracket = false;

  // scan forwards from cursor, looking for bracket (which would have function args in it)
  while (nEndChar < strWindowContents.GetLength () && 
         nEndChar < nStartChar + 30)
    {
    if (strWindowContents [nEndChar] == '(')
      {
      bFoundBracket = true;
      break;
      }
    // non-space means we have skipped the spaces basically, without finding a bracket
    else if (!isspace (strWindowContents [nEndChar]))
      break;
    else
      nEndChar++;
    }

  // if not found, assume a function call at least needs brackets
  // (first check if it *is* a function call and not something like sendto.script)
  if (dlg.m_strArgs.IsEmpty ())
    {
    // see if Lua function
    if (LuaFunctionsSet.find ((LPCTSTR) dlg.m_strResult) != LuaFunctionsSet.end ())
      dlg.m_strArgs = "( )";
    }   // end of no brackets

  // add in arguments if found and no arguments already
  if (!bFoundBracket && !dlg.m_strArgs.IsEmpty ())
    {
    dlg.m_strArgs = " " + dlg.m_strArgs;
    editctrl.ReplaceSel (dlg.m_strArgs, TRUE);
    // put cursor to right of LH bracket (add 2 to skip the space and the bracket)
    editctrl.SetSel (nStartChar + 2, nStartChar + 2);
    }

  // ensure text box has the focus if you click on the 'Complete' button in a dialog box
  // (otherwise the Complete button still has it)

  editctrl.SetFocus ();

  }
