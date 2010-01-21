// errors.h

// MUSHclient error codes - returned through OLE automation interfaces

enum
  {
  eOK = 0,                    // no error
  eWorldOpen = 30001,         // the world is already open
  eWorldClosed,               // the world is closed, this action cannot be performed - 30002
  eNoNameSpecified,           // no name has been specified where one is required - 30003
  eCannotPlaySound,           // the sound file could not be played - 30004
  eTriggerNotFound,           // the specified trigger name does not exist - 30005
  eTriggerAlreadyExists,      // attempt to add a trigger that already exists - 30006
  eTriggerCannotBeEmpty,      // the trigger "match" string cannot be empty - 30007
  eInvalidObjectLabel,        // the name of this object is invalid - 30008
  eScriptNameNotLocated,      // script name is not in the script file - 30009
  eAliasNotFound,             // the specified alias name does not exist - 30010
  eAliasAlreadyExists,        // attempt to add a alias that already exists - 30011
  eAliasCannotBeEmpty,        // the alias "match" string cannot be empty - 30012
  eCouldNotOpenFile,          // unable to open requested file - 30013
  eLogFileNotOpen,            // log file was not open - 30014
  eLogFileAlreadyOpen,        // log file was already open - 30015
  eLogFileBadWrite,           // bad write to log file - 30016
  eTimerNotFound,             // the specified timer name does not exist - 30017
  eTimerAlreadyExists,        // attempt to add a timer that already exists - 30018
  eVariableNotFound,          // attempt to delete a variable that does not exist - 30019
  eCommandNotEmpty,           // attempt to use SetCommand with a non-empty command window - 30020
  eBadRegularExpression,      // bad regular expression syntax - 30021
  eTimeInvalid,               // time given to AddTimer is invalid - 30022
  eBadMapItem,                // map item contains invalid characters, or is empty - 30023
  eNoMapItems,                // no items in mapper = 30024
  eUnknownOption,             // option name not found = 30025
  eOptionOutOfRange,          // new value for option is out of range = 30026
  eTriggerSequenceOutOfRange, // trigger sequence value invalid = 30027
  eTriggerSendToInvalid,      // where to send trigger text to is invalid = 30028
  eTriggerLabelNotSpecified,  // trigger label not specified/invalid for 'send to variable' = 30029
  ePluginFileNotFound,        // file name specified for plugin not found = 30030
  eProblemsLoadingPlugin,     // there was a parsing or other problem loading the plugin = 30031
  ePluginCannotSetOption,     // plugin is not allowed to set this option = 30032
  ePluginCannotGetOption,     // plugin is not allowed to get this option = 30033
  eNoSuchPlugin,              // plugin is not installed = 30034
  eNotAPlugin,                // only a plugin can do this = 30035
  eNoSuchRoutine,             // plugin does not support that routine = 30036
  ePluginDoesNotSaveState,    // plugin does not support saving state = 30037
  ePluginCouldNotSaveState,   // plugin could not save state (eg. no directory) = 30038
  ePluginDisabled,            // plugin is currently disabled = 30039
  eErrorCallingPluginRoutine, // could not call plugin routine = 30040
  eCommandsNestedTooDeeply,   // calls to "Execute" nested too deeply = 30041
  eCannotCreateChatSocket,    // unable to create socket for chat connection = 30042
  eCannotLookupDomainName,    // unable to do DNS (domain name) lookup for chat connection = 30043
  eNoChatConnections,         // no chat connections open = 30044
  eChatPersonNotFound,        // requested chat person not connected = 30045
  eBadParameter,              // general problem with a parameter to a script call = 30046
  eChatAlreadyListening,      // already listening for incoming chats = 30047
  eChatIDNotFound,            // chat session with that ID not found = 30048
  eChatAlreadyConnected,      // already connected to that server/port = 30049
  eClipboardEmpty,            // cannot get (text from the) clipboard = 30050
  eFileNotFound,              // cannot open the specified file = 30051
  eAlreadyTransferringFile,   // already transferring a file = 30052
  eNotTransferringFile,       // not transferring a file = 30053
  eNoSuchCommand,             // there is not a command of that name = 30054
  eArrayAlreadyExists,        // that array already exists = 30055
  eArrayDoesNotExist,         // that array does not exist = 30056
  eArrayNotEvenNumberOfValues,  // values to be imported into array are not in pairs = 30057
  eImportedWithDuplicates,    // import succeeded, however some values were overwritten = 30058
  eBadDelimiter,              // import/export delimiter must be a single character, other than backslash = 30059
  eSetReplacingExistingValue, // array element set, existing value overwritten = 30060
  eKeyDoesNotExist,           // array key does not exist = 30061
  eCannotImport,              // cannot import because cannot find unused temporary character = 30062
  eItemInUse,                 // cannot delete trigger/alias/timer because it is executing a script = 30063
  eSpellCheckNotActive,       // spell checker is not active = 30064

  // stuff for miniwindows

  eCannotAddFont,             // cannot create requested font = 30065
  ePenStyleNotValid,          // invalid settings for pen parameter = 30066
  eUnableToLoadImage,         // bitmap image could not be loaded = 30067
  eImageNotInstalled,         // image has not been loaded into window = 30068
  eInvalidNumberOfPoints,     // number of points supplied is incorrect = 30069
  eInvalidPoint,              // point is not numeric = 30070
  eHotspotPluginChanged,      // hotspot processing must all be in same plugin = 30071
  eHotspotNotInstalled,       // hotspot has not been defined for this window = 30072
  eNoSuchWindow,              // requested miniwindow does not exist = 30073
  eBrushStyleNotValid,        // invalid settings for brush parameter = 30074


#ifdef PANE
  ePaneAlreadyExists,         // specified pane cannot be created because it already exists = 30073
  ePaneDoesNotExist,          // specified pane does not exist = 30074
#endif

  // if you add things see lua_methods.cpp - there is a copy there

  };