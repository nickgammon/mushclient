// scriptingoptions.cpp - turn world options on/off from a script

#include "stdafx.h"
#include "MUSHclient.h"

#include "doc.h"
#include "scripting\errors.h"

#define NL "\r\n"

// Warning - when adding options check the SetOption function at the bottom of this
// file to make sure that some extra action is not needed if the option is changed
// (eg. changing the number of lines in the output window).


/*
 deprecated options not saved any more ...

  m_page_colour        
  m_whisper_colour     
  m_mail_colour        
  m_game_colour        
  m_chat_colour                             
  m_remove_channels1   
  m_remove_channels2   
  m_remove_pages       
  m_remove_whispers    
  m_remove_set         
  m_remove_mail        
  m_remove_game
  m_match_width
  m_timestamps

  */

static CMUSHclientDoc * pDoc = NULL;  // purely for #define below

#define O(arg) offsetof (CMUSHclientDoc, arg), sizeof (pDoc->arg)

//-----config name ----------------  default ---variable name ---- min, max, flags ---
tConfigurationNumericOption OptionsTable [] = {
{"accept_chat_connections",             false, O(m_bAcceptIncomingChatConnections)},               
{"alternative_inverse",                 false, O(m_bAlternativeInverse)},               
{"alt_arrow_recalls_partial",           false, O(m_bAltArrowRecallsPartial)},           
{"always_record_command_history",       false, O(m_bAlwaysRecordCommandHistory)},           
{"arrows_change_history",               true,  O(m_bArrowsChangeHistory)},              
{"arrow_keys_wrap",                     false, O(m_bArrowKeysWrap)},                    
{"arrow_recalls_partial",               false, O(m_bArrowRecallsPartial)},              
{"autosay_exclude_macros",              false, O(m_bExcludeMacros)},                    
{"autosay_exclude_non_alpha",           false, O(m_bExcludeNonAlpha)},                                      
{"auto_allow_files",                    false, O(m_bAutoAllowFiles)},                       
{"auto_allow_snooping",                 false, O(m_bAutoAllowSnooping)},                       
{"auto_copy_to_clipboard_in_html",      false, O(m_bAutoCopyInHTML)},                       
{"auto_pause",                          true,  O(m_bAutoFreeze)},                       
{"auto_repeat",                         false, O(m_bAutoRepeat)}, 
{"auto_resize_command_window",          false, O(m_bAutoResizeCommandWindow)},                      
{"auto_resize_minimum_lines",           1,     O(m_iAutoResizeMinimumLines), 1, 100},    
{"auto_resize_maximum_lines",           20,    O(m_iAutoResizeMaximumLines), 1, 100},    
{"auto_wrap_window_width",              false, O(m_bAutoWrapWindowWidth)}, 
{"carriage_return_clears_line",         false, O(m_bCarriageReturnClearsLine)},             
{"chat_foreground_colour",              RGB (255, 0, 0), O(m_cChatForegroundColour), 0, 0xFFFFFF, OPT_RGB_COLOUR }, 
{"chat_background_colour",              RGB (0, 0, 0),   O(m_cChatBackgroundColour), 0, 0xFFFFFF, OPT_RGB_COLOUR }, 
{"chat_max_lines_per_message",          0,     O(m_iMaxChatLinesPerMessage), 0, 10000 },
{"chat_max_bytes_per_message",          0,     O(m_iMaxChatBytesPerMessage), 0, LONG_MAX },
{"chat_port",                           DEFAULT_CHAT_PORT,  O(m_IncomingChatPort), 0, USHRT_MAX},            
//{"chat_sumchecks_to_sender",            false, O(m_bChatSumchecksToSender)},                       
{"confirm_before_replacing_typing",     true,  O(m_bConfirmBeforeReplacingTyping)},     
{"confirm_on_paste",                    true,  O(m_bConfirmOnPaste)},                                       
{"confirm_on_send",                     true,  O(m_bConfirmOnSend)},                    
{"connect_method",                      eNoAutoConnect, O(m_connect_now), eNoAutoConnect, eConnectTypeMax - 1},
{"copy_selection_to_clipboard",         false, O(m_bCopySelectionToClipboard)},
{"convert_ga_to_newline",               false, O(m_bConvertGAtoNewline)},
{"ctrl_n_goes_to_next_command",         false, O(m_bCtrlNGoesToNextCommand)},           
{"ctrl_p_goes_to_previous_command",     false, O(m_bCtrlPGoesToPreviousCommand)},       
{"ctrl_z_goes_to_end_of_buffer",        false, O(m_bCtrlZGoesToEndOfBuffer)},
{"custom_16_is_default_colour",         false, O(m_bCustom16isDefaultColour), 0, 0, OPT_UPDATE_VIEWS},           
{"detect_pueblo",                       true,  O(m_bPueblo)},
{"do_not_add_macros_to_command_history",false, O(m_bDoNotAddMacrosToCommandHistory)}, 
{"do_not_show_outstanding_lines",       false, O(m_bDoNotShowOutstandingLines)},
{"do_not_translate_iac_to_iac_iac",     false, O(m_bDoNotTranslateIACtoIACIAC)},                                      
{"disable_compression",                 false, O(m_bDisableCompression)},               
{"display_my_input",                    true,  O(m_display_my_input)},
{"double_click_inserts",                false, O(m_bDoubleClickInserts)},               
{"double_click_sends",                  false, O(m_bDoubleClickSends)}, 
{"echo_colour",                         0,     O(m_echo_colour), 0, MAX_CUSTOM, OPT_CUSTOM_COLOUR | OPT_UPDATE_VIEWS}, 
{"echo_hyperlink_in_output_window",     true,  O(m_bEchoHyperlinkInOutputWindow)},      
{"edit_script_with_notepad",            true,  O(m_bEditScriptWithNotepad)},            
{"enable_aliases",                      true,  O(m_enable_aliases)},                    
{"enable_auto_say",                     false, O(m_bEnableAutoSay)},                    
{"enable_beeps",                        true,  O(m_enable_beeps)},                      
{"enable_command_stack",                false, O(m_enable_command_stack)},              
{"enable_scripts",                      true,  O(m_bEnableScripts)},                    
{"enable_spam_prevention",              false, O(m_bEnableSpamPrevention)},                    
{"enable_speed_walk",                   false, O(m_enable_speed_walk)},                 
{"enable_timers",                       true,  O(m_bEnableTimers)},                     
{"enable_triggers",                     true,  O(m_enable_triggers)},                   
{"enable_trigger_sounds",               true,  O(m_enable_trigger_sounds)},             
{"escape_deletes_input",                false, O(m_bEscapeDeletesInput)},               
{"flash_taskbar_icon",                  false, O(m_bFlashIcon)},                        
{"history_lines",                       1000,  O(m_nHistoryLines), 20, 5000},       
{"hyperlink_adds_to_command_history",   true,  O(m_bHyperlinkAddsToCommandHistory)},   
{"hyperlink_colour",                    RGB (0, 128, 255), O(m_iHyperlinkColour), 0, 0xFFFFFF, OPT_RGB_COLOUR | OPT_UPDATE_VIEWS}, 
{"ignore_chat_colours",                 false,  O(m_bIgnoreChatColours)},               
{"ignore_mxp_colour_changes",           false, O(m_bIgnoreMXPcolourChanges)},             
{"indent_paras",                        true,  O(m_indent_paras)}, 
{"input_background_colour",             RGB (255, 255, 255), O(m_input_background_colour), 0, 0xFFFFFF, OPT_RGB_COLOUR | OPT_UPDATE_VIEWS}, 
{"input_font_height",                   12,    O(m_input_font_height), 1, 1000, OPT_UPDATE_VIEWS | OPT_UPDATE_INPUT_FONT}, 
{"input_font_italic",                   false, O(m_input_font_italic), 0, 0, OPT_UPDATE_VIEWS | OPT_UPDATE_INPUT_FONT},
{"input_font_weight",                   FW_DONTCARE,   O(m_input_font_weight), 0, 1000, OPT_UPDATE_VIEWS | OPT_UPDATE_INPUT_FONT},
{"input_font_charset",                  DEFAULT_CHARSET, O(m_input_font_charset), 0, 65536, OPT_UPDATE_VIEWS | OPT_UPDATE_INPUT_FONT},
{"input_text_colour",                   RGB (0, 0, 0), O(m_input_text_colour), 0, 0xFFFFFF, OPT_RGB_COLOUR | OPT_UPDATE_VIEWS},
{"keep_commands_on_same_line",          false, O(m_bKeepCommandsOnSameLine)},  
{"keypad_enable",                       true,  O(m_keypad_enable)},
{"line_information",                    true,  O(m_bLineInformation)},                                      
{"line_spacing",                        0,     O(m_iLineSpacing), 0, 100, OPT_UPDATE_VIEWS | OPT_UPDATE_OUTPUT_FONT}, 
{"log_html",                            false, O(m_bLogHTML)},                          
{"log_input",                           false, O(m_log_input)},
{"log_in_colour",                       false, O(m_bLogInColour)},           
{"log_notes",                           false, O(m_bLogNotes)},
{"log_output",                          true,  O(m_bLogOutput)},
{"log_raw",                             false, O(m_bLogRaw)},           
{"lower_case_tab_completion",           false, O(m_bLowerCaseTabCompletion)},           
{"map_failure_regexp",                  false, O(m_bMapFailureRegexp)},                 
{"max_output_lines",                    5000,  O(m_maxlines), 200, 500000, OPT_FIX_OUTPUT_BUFFER},            
{"mud_can_change_link_colour",          true,  O(m_bMudCanChangeLinkColour), 0, 0, OPT_SERVER_CAN_WRITE},           
{"mud_can_remove_underline",            false, O(m_bMudCanRemoveUnderline), 0, 0, OPT_SERVER_CAN_WRITE},            
{"mud_can_change_options",              true,  O(m_bMudCanChangeOptions)},            
{"mxp_debug_level",                     DBG_NONE, O(m_iMXPdebugLevel), 0, 4},      
{"naws",                                false, O(m_bNAWS)},                             
{"note_text_colour",                    4,     O(m_iNoteTextColour), 0, 0xFFFFFF, OPT_RGB_COLOUR | OPT_UPDATE_VIEWS}, 
{"no_echo_off",                         false, O(m_bNoEchoOff)},                        
{"output_font_height",                  12,    O(m_font_height), 1, 1000, OPT_UPDATE_VIEWS | OPT_UPDATE_OUTPUT_FONT}, 
{"output_font_weight",                  FW_DONTCARE, O(m_font_weight), 0, 1000, OPT_UPDATE_VIEWS | OPT_UPDATE_OUTPUT_FONT},
{"output_font_charset",                 DEFAULT_CHARSET, O(m_font_charset), 0, 65536, OPT_UPDATE_VIEWS | OPT_UPDATE_OUTPUT_FONT},
{"paste_commented_softcode",            false, O(m_bPasteCommentedSoftcode)},           
{"paste_delay",                         0,     O(m_nPasteDelay),  0, 100000},         
{"paste_delay_per_lines",               1,     O(m_nPasteDelayPerLines), 1, 100000}, 
{"paste_echo",                          false, O(m_bPasteEcho)},
{"pixel_offset",                        1,     O(m_iPixelOffset), 0, 20, OPT_UPDATE_VIEWS},        
{"port",                                4000,  O(m_port), 1, USHRT_MAX, OPT_PLUGIN_CANNOT_WRITE},                        
{"proxy_port",                          1080,  O(m_iProxyServerPort), 0, USHRT_MAX, OPT_PLUGIN_CANNOT_WRITE},                        
{"proxy_type",                          eProxyServerNone,     O(m_iSocksProcessing), eProxyServerNone, eProxyServerLast - 1, OPT_PLUGIN_CANNOT_WRITE},                        
{"re_evaluate_auto_say",                false, O(m_bReEvaluateAutoSay)},               
{"save_deleted_command",                false, O(m_bSaveDeletedCommand)},               
{"save_world_automatically",            false, O(m_bSaveWorldAutomatically)},           
{"script_reload_option",                eReloadConfirm, O(m_nReloadOption), 0, 2},
{"script_errors_to_output_window",      false, O(m_bScriptErrorsToOutputWindow) },       
{"send_echo",                           false, O(m_bSendEcho)},                         
//{"send_double_ff_for_hex_ff",           false, O(m_bSendDoubleHexFF)},            
{"send_file_commented_softcode",        false, O(m_bFileCommentedSoftcode)},            
{"send_file_delay",                     0,     O(m_nFileDelay), 0, 100000},          
{"send_file_delay_per_lines",           1,     O(m_nFileDelayPerLines), 1, 100000}, 
{"send_keep_alives",                    0,     O(m_bSendKeepAlives)}, 
{"send_mxp_afk_response",               true,  O(m_bSendMXP_AFK_Response)}, 
{"show_bold",                           false, O(m_bShowBold), 0, 0, OPT_UPDATE_VIEWS | OPT_UPDATE_OUTPUT_FONT},                         
{"show_connect_disconnect",             true,  O(m_bShowConnectDisconnect)},            
//{"show_grid_lines_in_list_views",       true,  O(m_bShowGridLinesInListViews)},            
{"show_italic",                         true,  O(m_bShowItalic), 0, 0, OPT_UPDATE_VIEWS | OPT_UPDATE_OUTPUT_FONT},                       
{"show_underline",                      true,  O(m_bShowUnderline), 0, 0, OPT_UPDATE_VIEWS | OPT_UPDATE_OUTPUT_FONT},                    
{"spam_line_count",                     20,    O(m_iSpamLineCount), 5, 500 },                    
{"speed_walk_delay",                    0,     O(m_iSpeedWalkDelay), 0, 30000, OPT_FIX_SPEEDWALK_DELAY},                  
{"spell_check_on_send",                 false, O(m_bSpellCheckOnSend)},                 
{"start_paused",                        false, O(m_bStartPaused)},  
{"tab_completion_lines",                200,   O(m_iTabCompletionLines), 1, 500000},
{"tab_completion_space",                false, O(m_bTabCompletionSpace)},
{"translate_backslash_sequences",       false, O(m_bTranslateBackslashSequences)},      
{"translate_german",                    false, O(m_bTranslateGerman)},                                      
{"underline_hyperlinks",                true,  O(m_bUnderlineHyperlinks), 0, 0, OPT_SERVER_CAN_WRITE},              
{"unpause_on_send",                     true,  O(m_bUnpauseOnSend)},                    
{"use_custom_link_colour",              true,  O(m_bUseCustomLinkColour), 0, 0, OPT_SERVER_CAN_WRITE},              
{"use_default_aliases",                 false, O(m_bUseDefaultAliases)},                
{"use_default_colours",                 false, O(m_bUseDefaultColours)},                
{"use_default_input_font",              false, O(m_bUseDefaultInputFont)},              
{"use_default_macros",                  false, O(m_bUseDefaultMacros)},                 
{"use_default_output_font",             false, O(m_bUseDefaultOutputFont)},             
{"use_default_timers",                  false, O(m_bUseDefaultTimers)},                 
{"use_default_triggers",                false, O(m_bUseDefaultTriggers)},               
{"use_mxp",                             eOnCommandMXP, O(m_iUseMXP), 0, 3, OPT_USE_MXP},
{"utf_8",                               false,  O(m_bUTF_8)},                    
{"validate_incoming_chat_calls",        false,  O(m_bValidateIncomingCalls)},               
{"warn_if_scripting_inactive",          true,  O(m_bWarnIfScriptingInactive)},            
{"wrap",                                true,  O(m_wrap)},             
{"wrap_column",                         80,    O(m_nWrapColumn), 20, MAX_LINE_WIDTH, OPT_FIX_WRAP_COLUMN},         
{"write_world_name_to_log",             true,  O(m_bWriteWorldNameToLog)},  

{NULL}   // end of table marker            

  };  // end of OptionsTable 

#define A(arg) offsetof (CMUSHclientDoc, arg)
                    
// first  optional argument: multiline flag
// second optional argument: preserve spaces flag (defaults to true for multiline)

tConfigurationAlphaOption AlphaOptionsTable  [] =
{
{"auto_log_file_name",            "",         A(m_strAutoLogFileName)},
{"auto_say_override_prefix",      "-",        A(m_strOverridePrefix)},
{"auto_say_string",               "say ",     A(m_strAutoSayString), OPT_KEEP_SPACES},
{"beep_sound",                    "",         A(m_strBeepSound)},
{"chat_file_save_directory",      "",         A(m_strChatSaveDirectory)},
{"chat_name",                     "",         A(m_strOurChatName)},
{"chat_message_prefix",           "",         A(m_strChatMessagePrefix), OPT_KEEP_SPACES},
{"command_stack_character",       ";",        A(m_strCommandStackCharacter), OPT_COMMAND_STACK},
{"connect_text",                  "",         A(m_connect_text), OPT_MULTLINE},
{"editor_window_name",            "",         A(m_strEditorWindowName) },
{"filter_aliases",                "",         A(m_strAliasesFilter), OPT_MULTLINE},
{"filter_timers",                 "",         A(m_strTimersFilter), OPT_MULTLINE},
{"filter_triggers",               "",         A(m_strTriggersFilter), OPT_MULTLINE},
{"filter_variables",              "",         A(m_strVariablesFilter), OPT_MULTLINE},
{"id",                            "",         A(m_strWorldID), OPT_WORLD_ID},
{"input_font_name",               "FixedSys", A(m_input_font_name), OPT_UPDATE_VIEWS | OPT_UPDATE_INPUT_FONT},
{"log_file_postamble",            "",         A(m_strLogFilePostamble), OPT_MULTLINE},
{"log_file_preamble",             "",         A(m_strLogFilePreamble), OPT_MULTLINE},
{"log_line_postamble_input",      "",         A(m_strLogLinePostambleInput), OPT_KEEP_SPACES},
{"log_line_postamble_notes",      "",         A(m_strLogLinePostambleNotes), OPT_KEEP_SPACES},
{"log_line_postamble_output",     "",         A(m_strLogLinePostambleOutput), OPT_KEEP_SPACES},
{"log_line_preamble_input",       "",         A(m_strLogLinePreambleInput), OPT_KEEP_SPACES},
{"log_line_preamble_notes",       "",         A(m_strLogLinePreambleNotes), OPT_KEEP_SPACES},
{"log_line_preamble_output",      "",         A(m_strLogLinePreambleOutput), OPT_KEEP_SPACES},
{"mapping_failure",               "Alas, you cannot go that way.", A(m_strMappingFailure), OPT_KEEP_SPACES},
{"name",                          "",         A(m_mush_name), OPT_PLUGIN_CANNOT_WRITE},
{"new_activity_sound",            NOSOUNDLIT, A(m_new_activity_sound)},
{"notes",                         "",         A(m_notes), OPT_MULTLINE},
{"on_mxp_close_tag",              "",         A(m_strOnMXP_CloseTag)},
{"on_mxp_error",                  "",         A(m_strOnMXP_Error)},
{"on_mxp_open_tag",               "",         A(m_strOnMXP_OpenTag)},
{"on_mxp_set_variable",           "",         A(m_strOnMXP_SetVariable)},
{"on_mxp_start",                  "",         A(m_strOnMXP_Start)},
{"on_mxp_stop",                   "",         A(m_strOnMXP_Stop)},
{"on_world_close",                "",         A(m_strWorldClose)},
{"on_world_save",                 "",         A(m_strWorldSave)},
{"on_world_connect",              "",         A(m_strWorldConnect)},
{"on_world_disconnect",           "",         A(m_strWorldDisconnect)},
{"on_world_get_focus",            "",         A(m_strWorldGetFocus)},
{"on_world_lose_focus",           "",         A(m_strWorldLoseFocus)},
{"on_world_open",                 "",         A(m_strWorldOpen)},
{"output_font_name",              "FixedSys", A(m_font_name), OPT_UPDATE_OUTPUT_FONT},
{"password",                      "",         A(m_password), OPT_PASSWORD | OPT_PLUGIN_CANNOT_RW},
{"paste_line_postamble",          "",         A(m_pasteline_postamble), OPT_KEEP_SPACES},
{"paste_line_preamble",           "",         A(m_pasteline_preamble), OPT_KEEP_SPACES},
{"paste_postamble",               "",         A(m_paste_postamble), OPT_MULTLINE},
{"paste_preamble",                "",         A(m_paste_preamble), OPT_MULTLINE},
{"player",                        "",         A(m_name), OPT_PLUGIN_CANNOT_WRITE},
{"proxy_server",                  "",         A(m_strProxyServerName), OPT_PLUGIN_CANNOT_WRITE},
{"proxy_username",                "",         A(m_strProxyUserName), OPT_PLUGIN_CANNOT_RW},
{"proxy_password",                "",         A(m_strProxyPassword), OPT_PASSWORD | OPT_PLUGIN_CANNOT_RW},
{"recall_line_preamble",          "",         A(m_strRecallLinePreamble), OPT_KEEP_SPACES},
{"script_editor",                 "notepad",  A(m_strScriptEditor)},
{"script_editor_argument",        "%file",    A(m_strScriptEditorArgument)},
{"script_filename",               "",         A(m_strScriptFilename)},
{"script_language",               "lua",      A(m_strLanguage)},
{"script_prefix",                 "",         A(m_strScriptPrefix)},
{"send_to_world_file_postamble",  "",         A(m_file_postamble), OPT_MULTLINE},
{"send_to_world_file_preamble",   "",         A(m_file_preamble), OPT_MULTLINE},
{"send_to_world_line_postamble",  "",         A(m_line_postamble)},
{"send_to_world_line_preamble",   "",         A(m_line_preamble)},
{"site",                          "",         A(m_server), OPT_PLUGIN_CANNOT_WRITE},
{"spam_message",                  "look",     A(m_strSpamMessage)},
{"speed_walk_filler",             "",         A(m_strSpeedWalkFiller), OPT_KEEP_SPACES},
{"speed_walk_prefix",             "#",        A(m_speed_walk_prefix), OPT_KEEP_SPACES},
{"tab_completion_defaults",       "",         A(m_strTabCompletionDefaults), OPT_MULTLINE},
{"terminal_identification",       "mushclient", A(m_strTerminalIdentification)},
{NULL}  // end of table marker
};

long CMUSHclientDoc::GetOptionIndex (LPCTSTR OptionName) 
  {

int iItem;
int iResult = FindBaseOption (OptionName, OptionsTable, iItem);

  if (iResult != eOK)
    return -1;
  
  return iItem;
  
  }   // end of CMUSHclientDoc::GetOptionIndex 


long CMUSHclientDoc::GetOption(LPCTSTR OptionName) 
{

long i = GetOptionIndex (OptionName);

  if (i == -1)
    return -1;   // not found

  if (m_CurrentPlugin &&
      (OptionsTable [i].iFlags & OPT_PLUGIN_CANNOT_READ))
    return -1;  // not available to plugin

  return GetOptionItem (i);


} // end of CMUSHclientDoc::GetOption

long CMUSHclientDoc::GetAlphaOptionIndex (LPCTSTR OptionName) 
  {

int iItem;
int iResult = FindBaseAlphaOption (OptionName, AlphaOptionsTable, iItem);

  if (iResult != eOK)
    return -1;
  
  return iItem;

  }   // end of CMUSHclientDoc::GetAlphaOptionIndex 


VARIANT CMUSHclientDoc::GetAlphaOption(LPCTSTR OptionName) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;   

long i = GetAlphaOptionIndex (OptionName);

  if (i == -1)
	  return vaResult;

  if (m_CurrentPlugin &&
      (AlphaOptionsTable [i].iFlags & OPT_PLUGIN_CANNOT_READ))
    return vaResult;  // not available to plugin

  SetUpVariantString (vaResult, GetAlphaOptionItem (i));
  return vaResult;

}

VARIANT CMUSHclientDoc::GetOptionList() 
{
  COleSafeArray sa;   // for list
  long i;

  // count them
  for (i = 0; OptionsTable [i].pName; i++)
    ;

  sa.CreateOneDim (VT_VARIANT, i);

  // put the numeric option names into the array
  for (i = 0; OptionsTable [i].pName; i++)
    {
    // the array must be a bloody array of variants, or VBscript kicks up
    COleVariant v (OptionsTable [i].pName);
    sa.PutElement (&i, &v);
    }      // end of looping through each option

	return sa.Detach ();
}

VARIANT CMUSHclientDoc::GetAlphaOptionList() 
{
  COleSafeArray sa;   // for list
  long i;

  // count them
  for (i = 0; AlphaOptionsTable [i].pName; i++)
    ;

  sa.CreateOneDim (VT_VARIANT, i);

  // put the alpha option names into the array
  for (i = 0; AlphaOptionsTable [i].pName; i++)
    {
    // the array must be a bloody array of variants, or VBscript kicks up
    COleVariant v (AlphaOptionsTable [i].pName);
    sa.PutElement (&i, &v);
    }      // end of looping through each option

	return sa.Detach ();
}

long SetBaseOptionItem (const int iItem,
                        tConfigurationNumericOption BaseOptionsTable [],
                        const int iItemCount,
                        char * pBase,
                        double Value,
                        bool & bChanged)

  {

  bChanged = false;

    // ensure in range
  if (iItem < 0 || iItem >= iItemCount)
    return eUnknownOption;

int iMinimum = BaseOptionsTable [iItem].iMinimum;
int iMaximum = BaseOptionsTable [iItem].iMaximum;

  // fix up default 0, 0 to be 0, 1
  if (iMinimum == 0 && iMaximum == 0)
      iMaximum = 1;

  if (Value < iMinimum || Value > iMaximum)
      return eOptionOutOfRange;

  // for custom colours, subtract 1 when we set it (-1 = no change (was 0) )
  //                                               (0 = colour 1 (was 1)   )
  if (BaseOptionsTable [iItem].iFlags & OPT_CUSTOM_COLOUR)
     Value--;

  // get variable address
  char * p = pBase + BaseOptionsTable [iItem].iOffset;

  if (BaseOptionsTable [iItem].iFlags & OPT_DOUBLE)
    {
    if (* (double *) p != Value)
      bChanged = true;
    * (double *) p = Value;
    }
  else
    {    // not double

    switch (BaseOptionsTable [iItem].iLength)
      {

      case 1:
        if (*p != (char) Value)
          bChanged = true;
        * p = (char) Value;
        break;

      case 2:
        if (* (short *) p != (short) Value)
          bChanged = true;
        * (short *) p = (short) Value;
        break;

      case 4:
        if (* (long *) p != Value)
          bChanged = true;
        * (long *) p = Value;
        break;

      case 8:
        if (* (__int64 *) p != Value)
          bChanged = true;
        * (__int64 *) p = Value; 
        break;

      default:
        return eUnknownOption;     // size unknown
      } // end of switch
    }   // not double

  return eOK;

  } // end of SetBaseOptionItem

long CMUSHclientDoc::SetOptionItem (const int iItem, 
                                    long Value, 
                                    const bool bDoSpecial,
                                    const bool bInclude) 
  {
bool bChanged;

long iResult = SetBaseOptionItem (iItem, 
                                  OptionsTable, 
                                  NUMITEMS (OptionsTable),
                                  (char *) this,
                                  Value,
                                  bChanged);

  if (iResult != eOK)
    return iResult;

  if (bChanged)
    SetModifiedFlag ();

  if (bInclude)
    {
    m_NumericConfiguration [iItem]->bInclude = true;
    m_NumericConfiguration [iItem]->iValue = Value;
    }

  // specials wanted?
  if (!bDoSpecial)
    return eOK;
  
// some special handling

  if (OptionsTable [iItem].iFlags & OPT_FIX_OUTPUT_BUFFER)
    if (m_pCurrentLine)     // a new world might not have a line yet
      FixUpOutputBuffer (Value);

  if (OptionsTable [iItem].iFlags & OPT_FIX_WRAP_COLUMN)
    {
    if (m_pCurrentLine)     // a new world might not have a line yet
      {
      // save current line text
      CString strLine = CString (m_pCurrentLine->text, m_pCurrentLine->len);

#ifdef USE_REALLOC
      m_pCurrentLine->text  = (char *) 
            realloc (m_pCurrentLine->text, 
                     MAX (m_pCurrentLine->len, Value) 
                     * sizeof (char));  
#else
      delete [] m_pCurrentLine->text;
      m_pCurrentLine->text = new char [MAX (m_pCurrentLine->len, Value)];
#endif

      // put text back
      memcpy (m_pCurrentLine->text, (LPCTSTR) strLine, m_pCurrentLine->len);
      ASSERT (m_pCurrentLine->text);
      }   // end of having a current line
    SendWindowSizes (Value);
    }

  if (OptionsTable [iItem].iFlags & OPT_FIX_SPEEDWALK_DELAY)
    SetSpeedWalkDelay ((short) Value);

  if (OptionsTable [iItem].iFlags & OPT_USE_MXP)
    {
    if (m_iUseMXP == eNoMXP && m_bMXP)
      MXP_Off (true);
    else if (m_iUseMXP == eUseMXP && !m_bMXP)
      MXP_On (false, true);   // not pueblo, manually on
    }

  if (OptionsTable [iItem].iFlags & OPT_UPDATE_INPUT_FONT)
    ChangeInputFont (m_input_font_height, 
                     m_input_font_name, 
                     m_input_font_weight, 
                     m_input_font_charset,
                     m_input_font_italic);

  if (OptionsTable [iItem].iFlags & OPT_UPDATE_OUTPUT_FONT)
    ChangeFont (m_font_height, 
                m_font_name, 
                m_font_weight, 
                m_font_charset,
                m_bShowBold,
                m_bShowItalic,
                m_bShowUnderline,
                m_iLineSpacing);

  if (OptionsTable [iItem].iFlags & OPT_UPDATE_VIEWS)
    UpdateAllViews (NULL);  


  return eOK;

  } // end of CMUSHclientDoc::SetOptionItem

// basic OptionItem retriever - used for worlds, triggers, aliases etc.

double GetBaseOptionItem (const int iItem,
                        tConfigurationNumericOption BaseOptionsTable [],
                        const int iItemCount,
                        char * pBase)
  {

  // ensure in range
  if (iItem < 0 || iItem >= iItemCount)
    return -1;

  // get variable address
  char * p = pBase + BaseOptionsTable [iItem].iOffset;
  double Value = -1;

  if (BaseOptionsTable [iItem].iFlags & OPT_DOUBLE)
    {
    Value =  * (double *) p;
    }
  else
    {   // not double

    switch (BaseOptionsTable [iItem].iLength)
      {

      case 1:
        Value = * p;
        break;

      case 2:
        Value =  * (unsigned short *) p;
        break;

      case 4:
        Value =  * (long *) p;
        break;

      case 8:
        Value =  * (__int64 *) p;   // warning - possible truncation!!!!
        break;

      } // end of switch
    } // not double

  // for custom colours, add 1 when we get it (-1 = no change (becomes 0) )
  //                                          (0 = colour 1 (becomes 1)   )
  if (BaseOptionsTable [iItem].iFlags & OPT_CUSTOM_COLOUR)
    {
    Value++;
    // however -1 was really 65535 so adding one made 65536, fix that up!
    if (Value == 65536)
      Value = 0;
    }

  // for booleans - make sure in range 0 to 1
  if (BaseOptionsTable [iItem].iMinimum == 0 &&
      BaseOptionsTable [iItem].iMaximum == 0)
      Value = Value != 0; // make sure true or false

  return Value;

  }  // end of CMUSHclientDoc::GetBaseOptionItem

long CMUSHclientDoc::GetOptionItem (const int iItem)
  {
  return GetBaseOptionItem (iItem, 
                            OptionsTable, 
                            NUMITEMS (OptionsTable),
                            (char *) this);  
  }


void CMUSHclientDoc::SetDefaults (const bool bDoSpecial)
  {
  for (int i = 0; OptionsTable [i].pName; i++)
      SetOptionItem (i, OptionsTable [i].iDefault, bDoSpecial, false);
  }   // end of CMUSHclientDoc::SetDefaults

long FindBaseOption (LPCTSTR OptionName, 
                    tConfigurationNumericOption BaseOptionsTable [],
                    int & iItem)
  {
CString strName = OptionName;

  strName.MakeLower();
  strName.TrimLeft ();
  strName.TrimRight ();

  for (iItem = 0; BaseOptionsTable [iItem].pName; iItem++)
    if (strName == BaseOptionsTable [iItem].pName)
      return eOK;   // found it

  return eUnknownOption;


  } // end of FindBaseOption

long FindBaseAlphaOption (LPCTSTR OptionName, 
                    tConfigurationAlphaOption BaseAlphaOptionsTable [],
                    int & iItem)
  {
CString strName = OptionName;

  strName.MakeLower();
  strName.TrimLeft ();
  strName.TrimRight ();

  for (iItem = 0; BaseAlphaOptionsTable [iItem].pName; iItem++)
    if (strName == BaseAlphaOptionsTable [iItem].pName)
      return eOK;   // found it

  return eUnknownOption;

  } // end of FindBaseAlphaOption


long CMUSHclientDoc::SetOption(LPCTSTR OptionName, 
                               long Value) 
{

int iItem;
int iResult = FindBaseOption (OptionName, OptionsTable, iItem);

  if (iResult != eOK)
    return iResult;

  if (m_CurrentPlugin &&
      (OptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    return ePluginCannotSetOption;

  return SetOptionItem (iItem, Value, true, false);

} // end of CMUSHclientDoc::SetOption

long SetBaseAlphaOptionItem (const int iItem,
                        tConfigurationAlphaOption BaseAlphaOptionsTable [],
                        const int iItemCount,
                        char * pBase,
                        CString & strValue,
                        bool & bChanged)

  {
  bChanged = false;

  // ensure in range
  if (iItem < 0 || iItem >= iItemCount)
    return eUnknownOption;

  // ensure they don't slip multi lines into single-line options
  if (!(BaseAlphaOptionsTable [iItem].iFlags & OPT_MULTLINE))
    {
    strValue.Replace ("\n", "");
    strValue.Replace ("\r", "");
    }

  // see if it is changing
  bChanged = * (CString *) (pBase + BaseAlphaOptionsTable [iItem].iOffset) != strValue;

  // set value
  * (CString *) (pBase + BaseAlphaOptionsTable [iItem].iOffset) = strValue;


  return eOK;

  } // end of SetBaseAlphaOptionItem

long CMUSHclientDoc::SetAlphaOptionItem (const int iItem, 
                                        LPCTSTR sValue, 
                                        const bool bDoSpecial,
                                        const bool bInclude)
  {


  CString strValue = sValue;

  if (AlphaOptionsTable [iItem].iFlags & OPT_COMMAND_STACK)
    {

    if (strValue.GetLength () > 1)
      return eOptionOutOfRange;

      if (strValue.IsEmpty ())
        {
        m_enable_command_stack = false;
        return eOptionOutOfRange;
        }

      if (!isprint (strValue [0]) || isspace (strValue [0]))
        {
        m_enable_command_stack = false;
        return eOptionOutOfRange;
        }

    } // end of command stack character

  if (AlphaOptionsTable [iItem].iFlags & OPT_WORLD_ID)
    {

    if (!strValue.IsEmpty ())
      if (strValue.GetLength () != PLUGIN_UNIQUE_ID_LENGTH)
        return eOptionOutOfRange;

    // make sure hex characters
    const char * p = strValue;
    for ( ; *p ; p++)
      if (!isxdigit (*p))
      return eOptionOutOfRange;

    strValue.MakeLower ();

    } // end of world id
  
bool bChanged;
long iResult = SetBaseAlphaOptionItem (iItem,
                        AlphaOptionsTable,
                        NUMITEMS (AlphaOptionsTable),
                        (char *) this,
                        strValue,
                        bChanged);

  if (iResult != eOK)
    return iResult;

  if (bChanged)
    SetModifiedFlag ();

  if (bInclude)
    {
    m_AlphaConfiguration [iItem]->bInclude = true;
    m_AlphaConfiguration [iItem]->sValue = strValue;
    }

  // specials wanted?
  if (!bDoSpecial)
    return eOK;


// any special tests here ...

  if (AlphaOptionsTable [iItem].iFlags & OPT_UPDATE_INPUT_FONT)
    ChangeInputFont (m_input_font_height, 
                     m_input_font_name, 
                     m_input_font_weight, 
                     m_input_font_charset,
                     m_input_font_italic);

  if (AlphaOptionsTable [iItem].iFlags & OPT_UPDATE_OUTPUT_FONT)
    ChangeFont (m_font_height, 
                m_font_name, 
                m_font_weight, 
                m_font_charset,
                m_bShowBold,
                m_bShowItalic,
                m_bShowUnderline,
                m_iLineSpacing);

  if (AlphaOptionsTable [iItem].iFlags & OPT_UPDATE_VIEWS)
    UpdateAllViews (NULL);  

  return eOK;

  }  // end of CMUSHclientDoc:SetAlphaOptionItem 
  
CString GetBaseAlphaOptionItem (const int iItem,
                                tConfigurationAlphaOption BaseAlphaOptionsTable [],
                                const int iItemCount,
                                char * pBase)
  {

  // ensure in range
  if (iItem < 0 || iItem >= iItemCount)
    return "";

  return * (CString *) (pBase + BaseAlphaOptionsTable [iItem].iOffset);

  }

CString CMUSHclientDoc::GetAlphaOptionItem (const int iItem)
  {

  return GetBaseAlphaOptionItem (iItem, 
                                 AlphaOptionsTable,
                                 NUMITEMS (AlphaOptionsTable),
                                 (char *) this);

  }  // end of CMUSHclientDoc::GetAlphaOptionItem

long CMUSHclientDoc::SetAlphaOption(LPCTSTR OptionName, LPCTSTR Value) 
  {

int iItem;
int iResult = FindBaseAlphaOption (OptionName, AlphaOptionsTable, iItem);

  if (iResult != eOK)
    return iResult;

  if (m_CurrentPlugin &&
      (AlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    return ePluginCannotSetOption;

  return SetAlphaOptionItem (iItem, Value, true, false);

} // end of CMUSHclientDoc::SetAlphaOption

void CMUSHclientDoc::SetAlphaDefaults (const bool bDoSpecial)
  {
  for (int i = 0; AlphaOptionsTable [i].pName; i++)
    if (AlphaOptionsTable [i].iFlags & OPT_WORLD_ID)
      SetAlphaOptionItem (i, ::GetUniqueID (), bDoSpecial, false);
    else
      SetAlphaOptionItem (i, AlphaOptionsTable [i].sDefault, bDoSpecial, false);

  }   // end of CMUSHclientDoc::SetAlphaDefaults


void CMUSHclientDoc::AllocateConfigurationArrays (void)
  {
int i; 

  m_NumericConfiguration.SetSize (NUMITEMS (OptionsTable));

  for (i = 0; i < NUMITEMS (OptionsTable); i++)
     m_NumericConfiguration.SetAt (i, new CNumericConfiguration);


  m_AlphaConfiguration.SetSize (NUMITEMS (AlphaOptionsTable));

  for (i = 0; i < NUMITEMS (AlphaOptionsTable); i++)
     m_AlphaConfiguration.SetAt (i, new CAlphaConfiguration);

  }

void CMUSHclientDoc::ResetConfigurationArrays (void)
  {
int i; 

  for (i = 0; i < NUMITEMS (OptionsTable); i++)
     m_NumericConfiguration [i]->Reset ();


  for (i = 0; i < NUMITEMS (AlphaOptionsTable); i++)
     m_AlphaConfiguration [i]->Reset ();

  }

void CMUSHclientDoc::DeleteConfigurationArrays (void)
  {
int i; 

  for (i = 0; i < NUMITEMS (OptionsTable); i++)
     delete m_NumericConfiguration  [i];

  m_NumericConfiguration.RemoveAll ();

  for (i = 0; i < NUMITEMS (AlphaOptionsTable); i++)
     delete m_AlphaConfiguration  [i];

  m_AlphaConfiguration.RemoveAll ();

  }