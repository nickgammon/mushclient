#include "stdafx.h"
#include "..\mainfrm.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "..\dialogs\FunctionListDlg.h"
#include "..\dialogs\CompleteWordDlg.h"

#define FUNCTIONS_FIRST_MENU 10000
#define FUNCTIONS_MENU_COUNT 30   // number of functions menu items we support

// select concat("'", name, "', ") FROM functions ORDER BY name; 
// then change ' to "

const char * sFunctions [] = {
    "Accelerator",  
    "AcceleratorTo",  
    "AcceleratorList",  
    "Activate",  
    "ActivateClient",  
    "ActivateNotepad",  
    "AddAlias",  
    "AddFont",
    "AddMapperComment",  
    "AddSpellCheckWord",
    "AddTimer",  
    "AddToMapper",  
    "AddTrigger",  
    "AddTriggerEx",  
    "AdjustColour",  
    "ANSI",  
    "AnsiNote",  
    "AppendToNotepad",  
    "ArrayClear",  
    "ArrayCount",  
    "ArrayCreate",  
    "ArrayDelete",  
    "ArrayDeleteKey",  
    "ArrayExists",  
    "ArrayExport",  
    "ArrayExportKeys",  
    "ArrayGet",  
    "ArrayGetFirstKey",  
    "ArrayGetLastKey",  
    "ArrayImport",  
    "ArrayKeyExists",  
    "ArrayListAll",  
    "ArrayListKeys",  
    "ArrayListValues",  
    "ArraySet",  
    "ArraySize",  
    "Base64Decode",  
    "Base64Encode", 
    "BlendPixel",
    "BoldColour",  
    "BroadcastPlugin",  
    "CallPlugin",
    "ChangeDir",
    "ChatAcceptCalls",  
    "ChatCall",  
    "ChatCallzChat",  
    "ChatDisconnect",  
    "ChatDisconnectAll",  
    "ChatEverybody",  
    "ChatGetID",  
    "ChatGroup",  
    "ChatID",  
    "ChatMessage",  
    "ChatNameChange",  
    "ChatNote",  
    "ChatPasteEverybody",  
    "ChatPasteText",  
    "ChatPeekConnections",  
    "ChatPersonal",  
    "ChatPing",  
    "ChatRequestConnections",  
    "ChatSendFile",  
    "ChatStopAcceptingCalls",  
    "ChatStopFileTransfer",  
    "CloseLog",  
    "CloseNotepad",  
    "ColourNameToRGB",  
    "ColourNote",  
    "ColourTell",  
    "Connect",  
    "CreateGUID",  
    "CustomColourBackground",  
    "CustomColourText",
    "DatabaseChanges",
    "DatabaseClose",
    "DatabaseColumnName",
    "DatabaseColumnNames",
    "DatabaseColumns",
    "DatabaseColumnText",
    "DatabaseColumnType",
    "DatabaseColumnValue",
    "DatabaseColumnValues",
    "DatabaseError",
    "DatabaseExec",
    "DatabaseFinalize",
    "DatabaseInfo",
    "DatabaseLastInsertRowid",
    "DatabaseList",
    "DatabaseOpen",
    "DatabasePrepare", 
    "DatabaseReset",
    "DatabaseStep",
    "DatabaseTotalChanges",
    "Debug",  
    "DeleteAlias",  
    "DeleteAliasGroup",  
    "DeleteAllMapItems",  
    "DeleteCommandHistory",  
    "DeleteGroup",  
    "DeleteLastMapItem", 
    "DeleteLines",
    "DeleteOutput",  
    "DeleteTemporaryAliases",  
    "DeleteTemporaryTimers",  
    "DeleteTemporaryTriggers",  
    "DeleteTimer",  
    "DeleteTimerGroup",  
    "DeleteTrigger",  
    "DeleteTriggerGroup",  
    "DeleteVariable",  
    "DiscardQueue",  
    "Disconnect",  
    "DoAfter",  
    "DoAfterNote",  
    "DoAfterSpecial",  
    "DoAfterSpeedWalk",
    "DoCommand",  
    "EchoInput",  
    "EditDistance",
    "EnableAlias",  
    "EnableAliasGroup",  
    "EnableGroup",  
    "EnableMapping",  
    "EnablePlugin",  
    "EnableTimer",  
    "EnableTimerGroup",  
    "EnableTrigger",  
    "EnableTriggerGroup",  
    "ErrorDesc",  
    "EvaluateSpeedwalk",  
    "Execute",  
    "ExportXML",  
    "FilterPixel",
    "FixupEscapeSequences",  
    "FixupHTML", 
    "FlashIcon",
    "FlushLog",
    "GenerateName",  
    "GetAlias",  
    "GetAliasInfo",  
    "GetAliasList",  
    "GetAliasOption",  
    "GetAliasWildcard",  
    "GetAlphaOption",  
    "GetAlphaOptionList",  
    "GetChatInfo",  
    "GetChatList",  
    "GetChatOption",  
    "GetClipboard",  
    "GetCommand",  
    "GetCommandList",  
    "GetConnectDuration",  
    "GetCurrentValue",  
    "GetCustomColourName",
    "GetDefaultValue",  
    "GetDeviceCaps",
    "GetEntity",  
    "GetFrame", 
    "GetGlobalOption",
    "GetGlobalOptionList",
    "GetHostAddress",  
    "GetHostName", 
    "GetInfo",  
    "GetInternalCommandsList",  
    "GetLineCount",  
    "GetLineInfo",  
    "GetLinesInBufferCount",  
    "GetLoadedValue",  
    "GetMainWindowPosition",  
    "GetMapColour",  
    "GetMappingCount",  
    "GetMappingItem",  
    "GetMappingString",  
    "GetNotepadLength",  
    "GetNotepadList",
    "GetNotepadText",  
    "GetNotepadWindowPosition",  
    "GetNotes",  
    "GetNoteStyle", 
    "GetOption",  
    "GetOptionList",  
    "GetPluginAliasInfo",  
    "GetPluginAliasList",  
    "GetPluginAliasOption",  
    "GetPluginID",  
    "GetPluginInfo",  
    "GetPluginList",  
    "GetPluginName",  
    "GetPluginTimerInfo",  
    "GetPluginTimerList",  
    "GetPluginTimerOption",  
    "GetPluginTriggerInfo",  
    "GetPluginTriggerList",  
    "GetPluginTriggerOption",  
    "GetPluginVariable",  
    "GetPluginVariableList",  
    "GetQueue",  
    "GetReceivedBytes",  
    "GetRecentLines",  
    "GetScriptTime",  
    "GetSelectionEndColumn",  
    "GetSelectionEndLine",  
    "GetSelectionStartColumn",  
    "GetSelectionStartLine",  
    "GetSentBytes", 
    "GetSoundStatus",
    "GetStyleInfo",  
    "GetSysColor",  
    "GetSystemMetrics",  
    "GetTimer",  
    "GetTimerInfo",  
    "GetTimerList",  
    "GetTimerOption",  
    "GetTrigger",  
    "GetTriggerInfo",  
    "GetTriggerList",  
    "GetTriggerOption",  
    "GetTriggerWildcard",  
    "GetUdpPort",  
    "GetUniqueID",  
    "GetUniqueNumber",  
    "GetVariable",  
    "GetVariableList",  
    "GetWorld",  
    "GetWorldById",  
    "GetWorldID",  
    "GetWorldIdList",  
    "GetWorldList",  
    "GetWorldWindowPosition",  
    "GetWorldWindowPositionX",  
    "GetXMLEntity",  
    "Hash", 
    "Help",
    "Hyperlink",  
    "ImportXML",  
    "Info",  
    "InfoBackground",  
    "InfoClear",  
    "InfoColour",  
    "InfoFont",  
    "IsAlias",  
    "IsConnected",  
    "IsLogOpen",  
    "IsPluginInstalled",  
    "IsTimer",  
    "IsTrigger",  
    "LoadPlugin",  
    "LogInput",  
    "LogNotes",  
    "LogOutput",  
    "LogSend",  
    "MakeRegularExpression",  
    "MapColour",  
    "MapColourList",  
    "Mapping",  
    "Metaphone",
    "MoveMainWindow",  
    "MoveNotepadWindow",  
    "MoveWorldWindow",  
    "MoveWorldWindowX",  
    "MtRand",  
    "MtSrand",  
    "NormalColour",  
    "Note",  
    "NoteColour",  
    "NoteColourBack",  
    "NoteColourFore",  
    "NoteColourName",  
    "NoteColourRGB",  
    "NoteHr",  
    "NotepadColour",  
    "NotepadFont",
    "NotepadReadOnly",
    "NotepadSaveMethod",
    "NoteStyle",  
    "Open",  
    "OpenBrowser",
    "OpenLog",  
    "PasteCommand",  
    "Pause",  
    "PickColour", 
    "PlaySound",
    "PluginSupports",  
    "PushCommand",  
    "Queue",  
    "ReadNamesFile",  
    "Redraw",  
    "ReloadPlugin",  
    "RemoveBacktracks",  
    "RemoveMapReverses", 
    "Repaint",
    "Replace",  
    "ReplaceNotepad",  
    "Reset",  
    "ResetIP",  
    "ResetStatusTime",  
    "ResetTimer",  
    "ResetTimers",  
    "ReverseSpeedwalk",  
    "RGBColourToName",  
    "Save",  
    "SaveNotepad",  
    "SaveState",  
    "SelectCommand", 
    "SetCommandWindowHeight",
    "Send",  
    "SendImmediate",  
    "SendNoEcho",
    "SendPkt",
    "SendPush", 
    "SendSpecial",
    "SendToNotepad",  
    "SetAliasOption",  
    "SetAlphaOption",
    "SetBackgroundColour",
    "SetBackgroundImage",
    "SetChanged",  
    "SetChatOption",  
    "SetClipboard",  
    "SetCommand", 
    "SetCommandSelection",
    "SetCursor",
    "SetCustomColourName",
    "SetEntity",  
    "SetForegroundImage",
    "SetInputFont",  
    "SetNotes",  
    "SetOption",  
    "SetOutputFont",  
    "SetStatus",  
    "SetTimerOption", 
    "SetToolBarPosition",
    "SetTriggerOption",  
    "SetVariable", 
    "SetWorldWindowStatus",
    "ShowInfoBar", 
    "ShiftTabCompleteItem",
    "Simulate",  
    "Sound",  
    "SpeedWalkDelay",  
    "SpellCheck",  
    "SpellCheckCommand", 
    "SpellCheckDlg",
    "StripANSI", 
    "StopSound",
    "Tell",  
    "TextRectangle",
    "Trace",  
    "TraceOut",  
    "TranslateGerman",
    "TranslateDebug",
    "Transparency",
    "Trim",  
    "UdpListen",  
    "UdpPortList",  
    "UdpSend",  
    "Version", 
    "WindowAddHotspot",
    "WindowArc",
    "WindowBezier",
    "WindowBlendImage",
    "WindowCircleOp",
    "WindowCreate",
    "WindowCreateImage",
    "WindowDelete",
    "WindowDeleteAllHotspots",
    "WindowDeleteHotspot",
    "WindowDragHandler",
    "WindowDrawImage",
    "WindowDrawImageAlpha",
    "WindowFilter",
    "WindowFont",
    "WindowFontInfo",
    "WindowFontList",
    "WindowGetImageAlpha",
    "WindowGetPixel",
    "WindowGradient",
    "WindowHotspotInfo",
    "WindowHotspotList",
    "WindowHotspotTooltip",
    "WindowImageFromWindow",
    "WindowImageInfo",
    "WindowImageList",
    "WindowImageOp",
    "WindowInfo",
    "WindowLine",
    "WindowList",
    "WindowLoadImage",
    "WindowMenu",
    "WindowMergeImageAlpha",
    "WindowPolygon",
    "WindowPosition",
    "WindowRectOp",
    "ScrollwheelHandler",
    "WindowSetPixel",
    "WindowShow",
    "WindowText",
    "WindowTextWidth",
    "WindowWrite",
    "WorldAddress",  
    "WorldName",  
    "WorldPort",
    "WriteLog", 
    
    ""    // end of table marker
  };

CString GetSelectedFunction (CString & strSelection, int & nStartChar, int & nEndChar)
  {

  CString strWord;

  if (!strSelection.IsEmpty ())
    {
    // if no selection try to get word under cursor


    if (nStartChar == nEndChar)
      {
      // if at end-of-file, go back a character
  //    if (nStartChar >= strSelection.GetLength ())
        nStartChar--;

      // scan backwards from cursor
      while (nStartChar >= 0 && 
             strWord.GetLength () < 30 &&
             (isalnum (strSelection [nStartChar]) || 
              strSelection [nStartChar] == '.' || 
              strSelection [nStartChar] == '-' || 
              strSelection [nStartChar] == '_')
             )
        {
        strWord = strSelection.Mid (nStartChar, 1) + strWord;
        nStartChar--;
        }

      nStartChar++;

      // scan forwards from cursor
      while (nEndChar < strSelection.GetLength () && 
             strWord.GetLength () < 30 &&
             (isalnum (strSelection [nEndChar]) || 
              strSelection [nEndChar] == '.' || 
              strSelection [nEndChar] == '-' || 
              strSelection [nEndChar] == '_')
             )
        {
        strWord = strWord + strSelection.Mid (nEndChar, 1);
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
        while (isspace (strSelection [nStartChar]) &&
               nEndChar > nStartChar)
          nStartChar++;

        // ditto for trailing spaces
        while (isspace (strSelection [nEndChar - 1]) &&
               (nEndChar - 1) > nStartChar)
          nEndChar--;

        strSelection = strSelection.Mid (nStartChar, nEndChar - nStartChar);
        strSelection.MakeLower ();
        strSelection.Replace ("@world.", ""); // ditto for @world.Note  (Ruby)
        strSelection.Replace ("world.", "");  // if they said world.note, just take note    
        strSelection.Replace ("$world->", ""); // ditto for $world->note  (Perl)
        bool bFunction = true;
        for (int i = 0; i < strSelection.GetLength (); i++)
          if (!isalnum (strSelection [i]))
            bFunction = false;
        if (bFunction)
          strWord = strSelection;
        }

      } // we have a selection

     }   // end of some selection supplied

  return strWord;

  } // end of GetSelectedFunction

void ShowFunctionslist (CString & strSelection, int nStartChar, int nEndChar, const bool bLua)
{

CString strWord = GetSelectedFunction (strSelection, nStartChar, nEndChar);

CFunctionListDlg dlg;

  dlg.m_bLua = bLua;
  dlg.m_strFilter = strWord;     // selected word from dialog/text window

  if (dlg.DoModal () == IDCANCEL || dlg.m_strResult.IsEmpty ())
    return;

  ShowHelp ("", dlg.m_strResult); // already has prefix

}


void CMainFrame::OnGameFunctionslist() 
  {
  ShowFunctionslist (CString (), true);
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
  CString strSelection;

  // find the selection range
  editctrl.GetSel(nStartChar, nEndChar);

  if (nEndChar < 0)
    nEndChar = nStartChar;

  // get window text
  editctrl.GetWindowText (strSelection);

  CString strWord = GetSelectedFunction (strSelection, nStartChar, nEndChar);

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

  // ensure text box has the focus if you click on the button
  editctrl.SetFocus ();

  }
