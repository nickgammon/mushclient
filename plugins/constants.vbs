<?xml version="1.0" encoding="UTF-8"?>

<!-- Constants For use in VB scripting -->

<!DOCTYPE script>
<script>
<![CDATA[
option explicit

' ----------------------------------------------------------
' Error codes returned by various functions
' ----------------------------------------------------------

const eOK = 0 ' No error
const eWorldOpen = 30001 ' The world is already open
const eWorldClosed = 30002 ' The world is closed, this action cannot be performed
const eNoNameSpecified = 30003 ' No name has been specified where one is required
const eCannotPlaySound = 30004 ' The sound file could not be played
const eTriggerNotFound = 30005 ' The specified trigger name does not exist
const eTriggerAlreadyExists = 30006 ' Attempt to add a trigger that already exists
const eTriggerCannotBeEmpty = 30007 ' The trigger "match" string cannot be empty
const eInvalidObjectLabel = 30008 ' The name of this object is invalid
const eScriptNameNotLocated = 30009 ' Script name is not in the script file
const eAliasNotFound = 30010 ' The specified alias name does not exist
const eAliasAlreadyExists = 30011 ' Attempt to add a alias that already exists
const eAliasCannotBeEmpty = 30012 ' The alias "match" string cannot be empty
const eCouldNotOpenFile = 30013 ' Unable to open requested file
const eLogFileNotOpen = 30014 ' Log file was not open
const eLogFileAlreadyOpen = 30015 ' Log file was already open
const eLogFileBadWrite = 30016 ' Bad write to log file
const eTimerNotFound = 30017 ' The specified timer name does not exist
const eTimerAlreadyExists = 30018 ' Attempt to add a timer that already exists
const eVariableNotFound = 30019 ' Attempt to delete a variable that does not exist
const eCommandNotEmpty = 30020 ' Attempt to use SetCommand with a non-empty command window
const eBadRegularExpression = 30021 ' Bad regular expression syntax
const eTimeInvalid = 30022 ' Time given to AddTimer is invalid
const eBadMapItem = 30023 ' Direction given to AddToMapper is invalid
const eNoMapItems = 30024 ' No items in mapper
const eUnknownOption = 30025 ' Option name not found
const eOptionOutOfRange = 30026 ' New value for option is out of range
const eTriggerSequenceOutOfRange = 30027 ' Trigger sequence value invalid
const eTriggerSendToInvalid = 30028 ' Where to send trigger text to is invalid
const eTriggerLabelNotSpecified = 30029 ' Trigger label not specified/invalid for 'send to variable'
const ePluginFileNotFound = 30030 ' File name specified for plugin not found
const eProblemsLoadingPlugin = 30031 ' There was a parsing or other problem loading the plugin
const ePluginCannotSetOption = 30032 ' Plugin is not allowed to set this option
const ePluginCannotGetOption = 30033 ' Plugin is not allowed to get this option
const eNoSuchPlugin = 30034 ' Requested plugin is not installed
const eNotAPlugin = 30035 ' Only a plugin can do this
const eNoSuchRoutine = 30036 ' Plugin does not support that subroutine (subroutine not in script)
const ePluginDoesNotSaveState = 30037 ' Plugin does not support saving state
const ePluginCouldNotSaveState = 30037 ' Plugin could not save state (eg. no state directory)
const ePluginDisabled = 30039 ' Plugin is currently disabled
const eErrorCallingPluginRoutine = 30040 ' Could not call plugin routine
const eCommandsNestedTooDeeply = 30041 ' Calls to "Execute" nested too deeply
const eCannotCreateChatSocket = 30042 ' Unable to create socket for chat connection
const eCannotLookupDomainName = 30043 ' Unable to do DNS (domain name) lookup for chat connection
const eNoChatConnections = 30044 ' No chat connections open
const eChatPersonNotFound = 30045 ' Requested chat person not connected
const eBadParameter = 30046 ' General problem with a parameter to a script call
const eChatAlreadyListening = 30047 ' Already listening for incoming chats
const eChatIDNotFound = 30048 ' Chat session with that ID not found
const eChatAlreadyConnected = 30049 ' Already connected to that server/port
const eClipboardEmpty = 30050 ' Cannot get (text from the) clipboard
const eFileNotFound = 30051 ' Cannot open the specified file
const eAlreadyTransferringFile = 30052 ' Already transferring a file
const eNotTransferringFile = 30053 ' Not transferring a file
const eNoSuchCommand = 30054 ' There is not a command of that name
const eArrayAlreadyExists = 30055  ' Chat session with that ID not found
const eArrayDoesNotExist = 30056  ' Already connected to that server/port
const eArrayNotEvenNumberOfValues = 30057  ' Cannot get (text from the) clipboard
const eImportedWithDuplicates = 30058  ' Cannot open the specified file
const eBadDelimiter = 30059  ' Already transferring a file
const eSetReplacingExistingValue = 30060  ' Not transferring a file
const eKeyDoesNotExist = 30061  ' There is not a command of that name
const eCannotImport = 30062  ' There is not a command of that name
const eItemInUse = 30063  ' Cannot delete trigger/alias/timer because it is executing a script
const eSpellCheckNotActive = 30064  ' Spell checker is not active
const eCannotAddFont = 30065          ' Cannot create requested font
const ePenStyleNotValid = 30066       ' Invalid settings for pen parameter
const eUnableToLoadImage = 30067      ' Bitmap image could not be loaded
const eImageNotInstalled = 30068      ' Image has not been loaded into window 
const eInvalidNumberOfPoints = 30069  ' Number of points supplied is incorrect 
const eInvalidPoint = 30070           ' Point is not numeric
const eHotspotPluginChanged = 30071   ' Hotspot processing must all be in same plugin
const eHotspotNotInstalled = 30072    ' Hotspot has not been defined for this window 
const eNoSuchWindow = 30073           ' Requested miniwindow does not exist
const eBrushStyleNotValid = 30074     ' Invalid settings for brush parameter


' ----------------------------------------------------------
' Flags for AddTrigger
' ----------------------------------------------------------

const eEnabled = 1 ' enable trigger 
const eOmitFromLog = 2 ' omit from log file 
const eOmitFromOutput = 4 ' omit trigger from output 
const eKeepEvaluating = 8 ' keep evaluating 
const eIgnoreCase = 16 ' ignore case when matching 
const eTriggerRegularExpression = 32 ' trigger uses regular expression 
const eExpandVariables = 512 ' expand variables like @direction 
const eReplace = 1024 ' replace existing trigger of same name 
const eLowercaseWildcard = 2048 ' wildcards forced to lower-case
const eTemporary = 16384 ' temporary - do not save to world file 
const eTriggerOneShot = 32768 ' one shot - delete after firing

' ----------------------------------------------------------
' Colours for AddTrigger
' ----------------------------------------------------------

const NOCHANGE = -1
const custom1 = 0
const custom2 = 1
const custom3 = 2 
const custom4 = 3
const custom5 = 4
const custom6 = 5
const custom7 = 6
const custom8 = 7
const custom9 = 8
const custom10 = 9
const custom11 = 10
const custom12 = 11
const custom13 = 12
const custom14 = 13
const custom15 = 14
const custom16 = 15
const custom_other = 16  ' triggers only

' ----------------------------------------------------------
' Flags for AddAlias
' ----------------------------------------------------------

' const eEnabled = 1  ' same as for AddTrigger
const eIgnoreAliasCase = 32 ' ignore case when matching 
const eOmitFromLogFile = 64 ' omit this alias from the log file 
const eAliasRegularExpression = 128 ' alias is regular expressions 
' const eExpandVariables = 512  ' same as for AddTrigger
' const eReplace = 1024  ' same as for AddTrigger
const eAliasSpeedWalk = 2048 ' interpret send string as a speed walk string 
const eAliasQueue = 4096 ' queue this alias for sending at the speedwalking delay interval 
const eAliasMenu = 8192 ' this alias appears on the alias menu 
' const eTemporary = 16384  ' same as for AddTrigger
const eAliasOneShot = 32768  ' one shot - delete after firing

' ----------------------------------------------------------
' Flags for AddTimer
' ----------------------------------------------------------

' const eEnabled = 1 ' same as for AddTrigger 
const eAtTime = 2 ' if not set, time is "every" 
const eOneShot = 4 ' if set, timer only fires once 
const eTimerSpeedWalk = 8 ' timer does a speed walk when it fires 
const eTimerNote = 16 ' timer does a world.note when it fires 
const eActiveWhenClosed = 32 ' timer fires even when world is disconnected
' const eReplace = 1024 ' same as for AddTrigger 
' const eTemporary = 16384 ' same as for AddTrigger
]]>            
 </script>
