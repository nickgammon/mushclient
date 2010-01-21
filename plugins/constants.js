<?xml version="1.0" encoding="UTF-8"?>

<!-- Constants For use in Jscript scripting -->

<!DOCTYPE script>
<script>
<![CDATA[
// ----------------------------------------------------------
// Error codes returned by various functions
// ----------------------------------------------------------

var eOK = 0; // No error
var eWorldOpen = 30001; // The world is already open
var eWorldClosed = 30002; // The world is closed, this action cannot be performed
var eNoNameSpecified = 30003; // No name has been specified where one is required
var eCannotPlaySound = 30004; // The sound file could not be played
var eTriggerNotFound = 30005; // The specified trigger name does not exist
var eTriggerAlreadyExists = 30006; // Attempt to add a trigger that already exists
var eTriggerCannotBeEmpty = 30007; // The trigger "match" string cannot be empty
var eInvalidObjectLabel = 30008; // The name of this object is invalid
var eScriptNameNotLocated = 30009; // Script name is not in the script file
var eAliasNotFound = 30010; // The specified alias name does not exist
var eAliasAlreadyExists = 30011; // Attempt to add a alias that already exists
var eAliasCannotBeEmpty = 30012; // The alias "match" string cannot be empty
var eCouldNotOpenFile = 30013; // Unable to open requested file
var eLogFileNotOpen = 30014; // Log file was not open
var eLogFileAlreadyOpen = 30015; // Log file was already open
var eLogFileBadWrite = 30016; // Bad write to log file
var eTimerNotFound = 30017; // The specified timer name does not exist
var eTimerAlreadyExists = 30018; // Attempt to add a timer that already exists
var eVariableNotFound = 30019; // Attempt to delete a variable that does not exist
var eCommandNotEmpty = 30020; // Attempt to use SetCommand with a non-empty command window
var eBadRegularExpression = 30021; // Bad regular expression syntax
var eTimeInvalid = 30022; // Time given to AddTimer is invalid
var eBadMapItem = 30023; // Direction given to AddToMapper is invalid
var eNoMapItems = 30024; // No items in mapper
var eUnknownOption = 30025; // Option name not found
var eOptionOutOfRange = 30026; // New value for option is out of range
var eTriggerSequenceOutOfRange = 30027; // Trigger sequence value invalid
var eTriggerSendToInvalid = 30028; // Where to send trigger text to is invalid
var eTriggerLabelNotSpecified = 30029; // Trigger label not specified/invalid for 'send to variable'
var ePluginFileNotFound = 30030; // File name specified for plugin not found
var eProblemsLoadingPlugin = 30031; // There was a parsing or other problem loading the plugin
var ePluginCannotSetOption = 30032; // Plugin is not allowed to set this option
var ePluginCannotGetOption = 30033; // Plugin is not allowed to get this option
var eNoSuchPlugin = 30034; // Requested plugin is not installed
var eNotAPlugin = 30035; // Only a plugin can do this
var eNoSuchRoutine = 30036; // Plugin does not support that subroutine (subroutine not in script)
var ePluginDoesNotSaveState = 30037; // Plugin does not support saving state
var ePluginCouldNotSaveState = 30037; // Plugin could not save state (eg. no state directory)
var ePluginDisabled = 30039; // Plugin is currently disabled
var eErrorCallingPluginRoutine = 30040; // Could not call plugin routine
var eCommandsNestedTooDeeply = 30041; // Calls to "Execute" nested too deeply
var eCannotCreateChatSocket = 30042; // Unable to create socket for chat connection
var eCannotLookupDomainName = 30043; // Unable to do DNS (domain name) lookup for chat connection
var eNoChatConnections = 30044; // No chat connections open
var eChatPersonNotFound = 30045; // Requested chat person not connected
var eBadParameter = 30046; // General problem with a parameter to a script call
var eChatAlreadyListening = 30047; // Already listening for incoming chats
var eChatIDNotFound = 30048; // Chat session with that ID not found
var eChatAlreadyConnected = 30049; // Already connected to that server/port
var eClipboardEmpty = 30050; // Cannot get (text from the) clipboard
var eFileNotFound = 30051; // Cannot open the specified file
var eAlreadyTransferringFile = 30052; // Already transferring a file
var eNotTransferringFile = 30053; // Not transferring a file
var eNoSuchCommand = 30054; // There is not a command of that name
var eArrayAlreadyExists = 30055;  // Chat session with that ID not found
var eArrayDoesNotExist = 30056;  // Already connected to that server/port
var eArrayNotEvenNumberOfValues = 30057;  // Cannot get (text from the) clipboard
var eImportedWithDuplicates = 30058;  // Cannot open the specified file
var eBadDelimiter = 30059;  // Already transferring a file
var eSetReplacingExistingValue = 30060;  // Not transferring a file
var eKeyDoesNotExist = 30061;  // There is not a command of that name
var eCannotImport = 30062;  // There is not a command of that name
var eItemInUse = 30063;     // Cannot delete trigger/alias/timer because it is executing a script
var eSpellCheckNotActive = 30064;    // Spell checker is not active
var eCannotAddFont = 30065;          // Cannot create requested font
var ePenStyleNotValid = 30066;       // Invalid settings for pen parameter
var eUnableToLoadImage = 30067;      // Bitmap image could not be loaded
var eImageNotInstalled = 30068;      // Image has not been loaded into window 
var eInvalidNumberOfPoints = 30069;  // Number of points supplied is incorrect 
var eInvalidPoint = 30070;           // Point is not numeric
var eHotspotPluginChanged = 30071;   // Hotspot processing must all be in same plugin
var eHotspotNotInstalled = 30072;    // Hotspot has not been defined for this window 
var eNoSuchWindow = 30073;           // Requested miniwindow does not exist
var eBrushStyleNotValid = 30074;     //  Invalid settings for brush parameter


// ----------------------------------------------------------
// Flags for AddTrigger
// ----------------------------------------------------------

var eEnabled = 1; // enable trigger 
var eOmitFromLog = 2; // omit from log file 
var eOmitFromOutput = 4; // omit trigger from output 
var eKeepEvaluating = 8; // keep evaluating 
var eIgnoreCase = 16; // ignore case when matching 
var eTriggerRegularExpression = 32; // trigger uses regular expression 
var eExpandVariables = 512; // expand variables like @direction 
var eReplace = 1024; // replace existing trigger of same name 
var eLowercaseWildcard = 2048;  // wildcards forced to lower-case
var eTemporary = 16384; // temporary - do not save to world file 
var eTriggerOneShot = 32768; // one shot - delete after firing

// ----------------------------------------------------------
// Colours for AddTrigger
// ----------------------------------------------------------

var NOCHANGE = -1;
var custom1 = 0;
var custom2 = 1;
var custom3 = 2; 
var custom4 = 3;
var custom5 = 4;
var custom6 = 5;
var custom7 = 6;
var custom8 = 7;
var custom9 = 8;
var custom10 = 9;
var custom11 = 10;
var custom12 = 11;
var custom13 = 12;
var custom14 = 13;
var custom15 = 14;
var custom16 = 15;
var custom_other = 16;  // triggers only

// ----------------------------------------------------------
// Flags for AddAlias
// ----------------------------------------------------------

// var eEnabled = 1; // same as for AddTrigger 
var eIgnoreAliasCase = 32; // ignore case when matching 
var eOmitFromLogFile = 64; // omit this alias from the log file 
var eAliasRegularExpression = 128; // alias is regular expressions 
var eExpandVariables = 512;  // same as for AddTrigger 
// var eReplace = 1024;  // same as for AddTrigger 
var eAliasSpeedWalk = 2048; // interpret send string as a speed walk string 
var eAliasQueue = 4096; // queue this alias for sending at the speedwalking delay interval 
var eAliasMenu = 8192; // this alias appears on the alias menu 
// var eTemporary = 16384;  // same as for AddTrigger 
var eAliasOneShot = 32768; // one shot - delete after firing


// ----------------------------------------------------------
// Flags for AddTimer
// ----------------------------------------------------------

// var eEnabled = 1; // same as for AddTrigger 
var eAtTime = 2; // if not set, time is "every" 
var eOneShot = 4; // if set, timer only fires once 
var eTimerSpeedWalk = 8; // timer does a speed walk when it fires 
var eTimerNote = 16; // timer does a world.note when it fires 
var eActiveWhenClosed = 32;  // timer fires even when world is disconnected
// var eReplace = 1024; // same as for AddTrigger
// var eTemporary = 16384; // same as for AddTrigger

]]>            
 </script>
