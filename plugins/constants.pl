<?xml version="1.0" encoding="UTF-8"?>

<!-- Constants For use in Perlscript scripting -->

<!DOCTYPE script>
<script>
<![CDATA[
# ----------------------------------------------------------
# Error codes returned by various functions
# ----------------------------------------------------------

my $eOK = 0; # No error
my $eWorldOpen = 30001; # The world is already open
my $eWorldClosed = 30002; # The world is closed, this action cannot be performed
my $eNoNameSpecified = 30003; # No name has been specified where one is required
my $eCannotPlaySound = 30004; # The sound file could not be played
my $eTriggerNotFound = 30005; # The specified trigger name does not exist
my $eTriggerAlreadyExists = 30006; # Attempt to add a trigger that already exists
my $eTriggerCannotBeEmpty = 30007; # The trigger "match" string cannot be empty
my $eInvalidObjectLabel = 30008; # The name of this object is invalid
my $eScriptNameNotLocated = 30009; # Script name is not in the script file
my $eAliasNotFound = 30010; # The specified alias name does not exist
my $eAliasAlreadyExists = 30011; # Attempt to add a alias that already exists
my $eAliasCannotBeEmpty = 30012; # The alias "match" string cannot be empty
my $eCouldNotOpenFile = 30013; # Unable to open requested file
my $eLogFileNotOpen = 30014; # Log file was not open
my $eLogFileAlreadyOpen = 30015; # Log file was already open
my $eLogFileBadWrite = 30016; # Bad write to log file
my $eTimerNotFound = 30017; # The specified timer name does not exist
my $eTimerAlreadyExists = 30018; # Attempt to add a timer that already exists
my $eVariableNotFound = 30019; # Attempt to delete a variable that does not exist
my $eCommandNotEmpty = 30020; # Attempt to use SetCommand with a non-empty command window
my $eBadRegularExpression = 30021; # Bad regular expression syntax
my $eTimeInvalid = 30022; # Time given to AddTimer is invalid
my $eBadMapItem = 30023; # Direction given to AddToMapper is invalid
my $eNoMapItems = 30024; # No items in mapper
my $eUnknownOption = 30025; # Option name not found
my $eOptionOutOfRange = 30026; # New value for option is out of range
my $eTriggerSequenceOutOfRange = 30027; # Trigger sequence value invalid
my $eTriggerSendToInvalid = 30028; # Where to send trigger text to is invalid
my $eTriggerLabelNotSpecified = 30029; # Trigger label not specified/invalid for 'send to variable'
my $ePluginFileNotFound = 30030; # File name specified for plugin not found
my $eProblemsLoadingPlugin = 30031; # There was a parsing or other problem loading the plugin
my $ePluginCannotSetOption = 30032; # Plugin is not allowed to set this option
my $ePluginCannotGetOption = 30033; # Plugin is not allowed to get this option
my $eNoSuchPlugin = 30034; # Requested plugin is not installed
my $eNotAPlugin = 30035; # Only a plugin can do this
my $eNoSuchRoutine = 30036; # Plugin does not support that subroutine (subroutine not in script)
my $ePluginDoesNotSaveState = 30037; # Plugin does not support saving state
my $ePluginCouldNotSaveState = 30037; # Plugin could not save state (eg. no state directory)
my $ePluginDisabled = 30039; # Plugin is currently disabled
my $eErrorCallingPluginRoutine = 30040; # Could not call plugin routine
my $eCommandsNestedTooDeeply = 30041; # Calls to "Execute" nested too deeply
my $eCannotCreateChatSocket = 30042; # Unable to create socket for chat connection
my $eCannotLookupDomainName = 30043; # Unable to do DNS (domain name) lookup for chat connection
my $eNoChatConnections = 30044; # No chat connections open
my $eChatPersonNotFound = 30045; # Requested chat person not connected
my $eBadParameter = 30046; # General problem with a parameter to a script call
my $eChatAlreadyListening = 30047; # Already listening for incoming chats
my $eChatIDNotFound = 30048; # Chat session with that ID not found
my $eChatAlreadyConnected = 30049; # Already connected to that server/port
my $eClipboardEmpty = 30050; # Cannot get (text from the) clipboard
my $eFileNotFound = 30051; # Cannot open the specified file
my $eAlreadyTransferringFile = 30052; # Already transferring a file
my $eNotTransferringFile = 30053; # Not transferring a file
my $eNoSuchCommand = 30054; # There is not a command of that name
my $eArrayAlreadyExists = 30055;  # Chat session with that ID not found
my $eArrayDoesNotExist = 30056;  # Already connected to that server/port
my $eArrayNotEvenNumberOfValues = 30057;  # Cannot get (text from the) clipboard
my $eImportedWithDuplicates = 30058;  # Cannot open the specified file
my $eBadDelimiter = 30059;  # Already transferring a file
my $eSetReplacingExistingValue = 30060;  # Not transferring a file
my $eKeyDoesNotExist = 30061;  # There is not a command of that name
my $eCannotImport = 30062;  # There is not a command of that name
my $eItemInUse = 30063;  #Cannot delete trigger/alias/timer because it is executing a script
my $eSpellCheckNotActive = 30064;    # Spell checker is not active
my $eCannotAddFont = 30065;          # Cannot create requested font
my $ePenStyleNotValid = 30066;       # Invalid settings for pen parameter
my $eUnableToLoadImage = 30067;      # Bitmap image could not be loaded
my $eImageNotInstalled = 30068;      # Image has not been loaded into window 
my $eInvalidNumberOfPoints = 30069;  # Number of points supplied is incorrect 
my $eInvalidPoint = 30070;           # Point is not numeric
my $eHotspotPluginChanged = 30071;   # Hotspot processing must all be in same plugin
my $eHotspotNotInstalled = 30072;    # Hotspot has not been defined for this window 
my $eNoSuchWindow = 30073;           # Requested miniwindow does not exist
my $eBrushStyleNotValid = 30074;     # Invalid settings for brush parameter


# ----------------------------------------------------------
# Flags for AddTrigger
# ----------------------------------------------------------

my $eEnabled = 1; # enable trigger 
my $eOmitFromLog = 2; # omit from log file 
my $eOmitFromOutput = 4; # omit trigger from output 
my $eKeepEvaluating = 8; # keep evaluating 
my $eIgnoreCase = 16; # ignore case when matching 
my $eTriggerRegularExpression = 32; # trigger uses regular expression 
my $eExpandVariables = 512; # expand variables like @direction 
my $eReplace = 1024; # replace existing trigger of same name 
my $eLowercaseWildcard = 2048; # wildcards forced to lower-case
my $eTemporary = 16384; # temporary - do not save to world file 
my $eTriggerOneShot = 32768; # one shot - delete after firing

# ----------------------------------------------------------
# Colours for AddTrigger
# ----------------------------------------------------------

my $NOCHANGE = -1;
my $custom1 = 0;
my $custom2 = 1;
my $custom3 = 2; 
my $custom4 = 3;
my $custom5 = 4;
my $custom6 = 5;
my $custom7 = 6;
my $custom8 = 7;
my $custom9 = 8;
my $custom10 = 9;
my $custom11 = 10;
my $custom12 = 11;
my $custom13 = 12;
my $custom14 = 13;
my $custom15 = 14;
my $custom16 = 15;
my $custom_other = 16;  # triggers only

# ----------------------------------------------------------
# Flags for AddAlias
# ----------------------------------------------------------

# my $eEnabled = 1; # same as for AddTrigger 
my $eIgnoreAliasCase = 32; # ignore case when matching 
my $eOmitFromLogFile = 64; # omit this alias from the log file 
my $eAliasRegularExpression = 128; # alias is regular expressions 
# my $eExpandVariables = 512; # same as for AddTrigger 
# my $eReplace = 1024; # same as for AddTrigger 
my $eAliasSpeedWalk = 2048; # interpret send string as a speed walk string 
my $eAliasQueue = 4096; # queue this alias for sending at the speedwalking delay interval 
my $eAliasMenu = 8192; # this alias appears on the alias menu 
# my $eTemporary = 16384; # same as for AddTrigger
my $eAliasOneShot = 32768; # one shot - delete after firing

# ----------------------------------------------------------
# Flags for AddTimer
# ----------------------------------------------------------

# my $eEnabled = 1; # same as for AddTrigger
my $eAtTime = 2; # if not set, time is "every" 
my $eOneShot = 4; # if set, timer only fires once 
my $eTimerSpeedWalk = 8; # timer does a speed walk when it fires 
my $eTimerNote = 16; # timer does a world.note when it fires 
my $eActiveWhenClosed = 32;  # timer fires even when world is disconnected
# my $eReplace = 1024; # same as for AddTrigger
# my $eTemporary = 16384; # same as for AddTrigger

]]>            
 </script>
