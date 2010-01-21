#pragma once

// MXP actions (for elements)

enum
  {
  MXP_ACTION_SEND,        // eg. <send href="go west"> west 
  MXP_ACTION_BOLD,        // bold                           
  MXP_ACTION_UNDERLINE,   // underline                      
  MXP_ACTION_ITALIC,      // italic                         
  MXP_ACTION_COLOR,       // eg. <color fore=red back=blue>              
  MXP_ACTION_VERSION,      // version request
  MXP_ACTION_FONT,        // Font appearance             
  MXP_ACTION_SOUND,       // play sound                  
  MXP_ACTION_USER,        // send username               
  MXP_ACTION_PASSWORD,    // send password               
  MXP_ACTION_RELOCATE,    // causes a new connect to open
  MXP_ACTION_FRAME,       // frame                       
  MXP_ACTION_DEST,        // destination frame           
  MXP_ACTION_IMAGE,       // show image                  
  MXP_ACTION_FILTER,      // sound/image filter          
  MXP_ACTION_HYPERLINK,   // Hyperlink (secure)      
  MXP_ACTION_BR,          // Hard Line break (secure)
  MXP_ACTION_H1,          // Level 1 heading (secure)
  MXP_ACTION_H2,          // Level 2 heading (secure)
  MXP_ACTION_H3,          // Level 3 heading (secure)
  MXP_ACTION_H4,          // Level 4 heading (secure)
  MXP_ACTION_H5,          // Level 5 heading (secure)
  MXP_ACTION_H6,          // Level 6 heading (secure)
  MXP_ACTION_HR,          // Horizontal rule (secure) 
  MXP_ACTION_NOBR,        // non-breaking newline     
  MXP_ACTION_P,           // Paragraph break (secure) 
  MXP_ACTION_STRIKE,      // Strikethrough          
  MXP_ACTION_SCRIPT,      // Client script (secure) 
  MXP_ACTION_SMALL,       // Small text            
  MXP_ACTION_TT,          // Non-proportional font 
  MXP_ACTION_UL,          // Unordered list 
  MXP_ACTION_OL,          // Ordered list 
  MXP_ACTION_LI,          // List item 
  MXP_ACTION_SAMP,        // Sample text
  MXP_ACTION_CENTER,      // Centre text
  MXP_ACTION_HIGH,        // Highlight text
  MXP_ACTION_VAR,         // Set variable
  MXP_ACTION_AFK,         // AFK - away from keyboard time


  // recent
  MXP_ACTION_GAUGE,       // gauge                  
  MXP_ACTION_STAT,        // status
  MXP_ACTION_EXPIRE,      // expire

  // non-standard yet
  MXP_ACTION_RESET,       // close all open tags 
  MXP_ACTION_MXP,         // MXP command (eg. MXP OFF) 
  MXP_ACTION_SUPPORT,     // what commands we support
  MXP_ACTION_OPTION,      // client options set
  MXP_ACTION_RECOMMEND_OPTION, // server sets option

  // Pueblo

  MXP_ACTION_PRE,         // Preformatted text
  MXP_ACTION_BODY, 
  MXP_ACTION_HEAD, 
  MXP_ACTION_HTML, 
  MXP_ACTION_TITLE,
  MXP_ACTION_IMG,
  MXP_ACTION_XCH_PAGE,
  MXP_ACTION_XCH_PANE,
  
  };

// title for notepad window for MXP errors
#define MXP_ERROR_WINDOW "MXP Messages"

// structure for loading elements

// flags for tags

#define TAG_OPEN      0x01   // tag is an open one (otherwise secure)
#define TAG_COMMAND   0x02   // tag is a command (doesn't have closing tag)
#define TAG_PUEBLO    0x04   // tag is Pueblo-only
#define TAG_MXP       0x08   // tag is MXP-only
#define TAG_NO_RESET  0x10   // not closed by reset (eg. body)
#define TAG_NOT_IMP   0x20   // not really implemented (for <supports> tag )

typedef struct
  {
  char *  pName;     // name, eg. "bold"
  int     iFlags;    // see above, secure or command flags
  int     iAction;   // action, eg. MXP_ACTION_BOLD
  char *  pArgs;     // supported arguments
  }  tMXP_element;

// structure for loading colours

typedef struct
  {
  char      * pName;       // name, eg. "red"
  COLORREF  iColour;     // colour, eg. 0xff0000
  }  tMXP_colour;

// MXP error numbers

// in the examples the sequence "..." means "some string of characters"

enum {

// error level messages

  errMXP_Unknown = 1000,        // should not get this
  errMXP_UnterminatedElement,   // eg. < ... \n
  errMXP_UnterminatedComment,   // eg. <!-- ... \n
  errMXP_UnterminatedEntity,    // eg. & ... \n
  errMXP_UnterminatedQuote,     // eg. < ' ... \n
  errMXP_EmptyElement,          // eg. <>
  errMXP_ElementTooShort,       // eg. <!>
  errMXP_InvalidEntityName,     // eg. &*;
  errMXP_DefinitionWhenNotSecure,   // eg. <!ELEMENT ... > in open mode
  errMXP_InvalidElementName,    // eg. < 2345 >  or </ 2345 >
  errMXP_InvalidDefinition,     // ie. not <!ELEMENT ...> or <!ENTITY ...>
  errMXP_CannotRedefineElement, // cannot redefine inbuilt element
  errMXP_NoTagInDefinition,     // no < in element definition, eg.
                                // <!ELEMENT foo 'bold' >  (should be '<bold>' )
  errMXP_UnexpectedDefinitionSymbol,  // eg. <!ELEMENT foo '<<bold>' >
  errMXP_NoClosingDefinitionQuote,    // eg. <!ELEMENT foo '<send "west>' >
  errMXP_NoClosingDefinitionTag,  // eg. <!ELEMENT foo '<bold' >
  errMXP_NoInbuiltDefinitionTag,  // defining unknown tag, eg. <!ELEMENT foo '<bar>' >   
  errMXP_NoDefinitionTag,         // eg. <!ELEMENT foo '<>' >
  errMXP_BadVariableName,       // variable name in FLAG does not meet MUSHclient rules
  errMXP_UnknownElementInAttlist, // ATTLIST for undefined element name
  errMXP_CannotRedefineEntity,  // cannot redefine inbuilt entity
  errMXP_NoClosingSemicolon,    // eg. <!ENTITY foo &quot >
  errMXP_UnexpectedEntityArguments, // eg. <!ENTITY foo 'bar' xxxx >
  errMXP_UnknownElement,        // eg. <blah>
  errMXP_ElementWhenNotSecure,  // eg. <send> in open mode
  errMXP_NoClosingSemicolonInArgument,  // eg. <!ELEMENT foo '<send &bar>'>
  errMXP_ClosingUnknownTag,     // closing tag we don't recognise
  errMXP_UnknownColour,         // argument to COLOR or FONT not recognised colour
  errMXP_InvalidEntityNumber,   // eg. &#xxx;
  errMXP_DisallowedEntityNumber,  // eg. &#5000;
  errMXP_UnknownEntity,         // eg. &foo;
  errMXP_InvalidArgumentName,   // eg. <color 123=blue>  (123 is invalid)
  errMXP_NoArgument,            // eg. <font color=>
  errMXP_PuebloOnly,            // using Pueblo element in MXP mode
  errMXP_MXPOnly,               // using MXP element in Pueblo mode
  errMXP_DefinitionAttemptInPueblo, // Pueblo does not support <!ELEMENT> or <!ENTITY>
  errMXP_InvalidSupportArgument,  // invalid argument to <support> tag
  errMXP_InvalidOptionArgument,  // invalid argument to <option> tag
  errMXP_DefinitionCannotCloseElement,  // eg. <!ELEMENT foo '</bold>' >
  errMXP_DefinitionCannotDefineElement,  // eg. <!ELEMENT foo '<!ELEMENT>' >
  errMXP_CannotChangeOption,      // cannot change option with <recommend_option>
  errMXP_OptionOutOfRange,        // option not in acceptable range


// warning level messages

  wrnMXP_ReplacingElement = 5000, // replacing previously-defined element
  wrnMXP_ManyOutstandingTags,     // too many unclosed tags 
  wrnMXP_ArgumentNotSupplied,     // argument needed but not supplied or blank 
  wrnMXP_ArgumentsToClosingTag,   // eg. </send bar > 
  wrnMXP_OpenTagBlockedBySecureTag, // when closing an open tag secure tag blocks it 
  wrnMXP_OpenTagNotThere,         // eg. </bold> when no opening tag 
  wrnMXP_TagOpenedInSecureMode,   // cannot close tag - it was opened in secure mode 
  wrnMXP_ClosingOutOfSequenceTag, // eg. <b> <i> </b>
  wrnMXP_OpenTagNotInOutputBuffer,  // opening tag no longer in buffer 
  wrnMXP_CharacterNameRequestedButNotDefined, // no name known - cannot reply to <user>
  wrnMXP_PasswordNotSent,         // password requested too late in session
  wrnMXP_PasswordRequestedButNotDefined,  // no password known - cannot reply to <pass>
  wrnMXP_TagNotImplemented,       // MXP tag known but not implemented 
  wrnMXP_OpenTagClosedAtEndOfLine,  // tag closed because \n received 
  wrnMXP_TagClosedAtReset,        // tag closed because <reset> received
  wrnMXP_UnusedArgument,          // argument supplied but not used, eg. <bold blue> 
  wrnMXP_NotStartingPueblo,       // Pueblo string received but not activating it

// information level messages

  infoMXP_VersionSent   = 10000,  // <version> response sent
  infoMXP_CharacterNameSent,      // <user> response sent 
  infoMXP_PasswordSent,           // <pass> response sent 
  infoMXP_ScriptCollectionStarted,  // collecting <script> text 
  infoMXP_ScriptCollectionCompleted,  // finished collecting <script> 
  infoMXP_ResetReceived,          // <reset> received 
  infoMXP_off,                    // mxp turned off 
  infoMXP_on,                     // mxp turned on 
  infoMXP_ModeChange,             // mxp security mode change 
  infoMXP_SupportsSent,           // mxp <supports> tag sent
  infoMXP_OptionsSent,            // mxp <options> tag sent
  infoMXP_AFKSent,                // mxp <afk> tag sent
  infoMXP_OptionChanged,          // client option changed by <recommend_option>

// other messages ("all" category) - pretty spammy

  msgMXP_CollectedElement = 20000,  // received < ... >
  msgMXP_CollectedEntity,           // received & ... ; 
  msgMXP_GotDefinition,             // received <!ELEMENT ...> or <!ENTITY ...> 

  };