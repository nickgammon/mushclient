# Example of coding script routines for MUSHclient in PerlScript

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
my $eArrayAlreadyExists = 30055; # That array already exists
my $eBadKeyName = 30056; # That name is not permitted for a key
my $eArrayDoesNotExist = 30056; # That array does not exist
my $eArrayNotEvenNumberOfValues = 30057; # Values to be imported into array are not in pairs
my $eImportedWithDuplicates = 30058; # Import succeeded, however some values were overwritten
my $eBadDelimiter = 30059; # Import/export delimiter must be a single character, other than backslash
my $eSetReplacingExistingValue = 30060; # Array element set, existing value overwritten
my $eKeyDoesNotExist = 30061; # Array key does not exist
my $eCannotImport = 30062; # Cannot import because cannot find unused temporary character


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
my $eTemporary = 16384; # temporary - do not save to world file 

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
my $eTemporary = 16384; # temporary - do not save to world file

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

# ----------------------------------------------------------
# Example showing iterating through all triggers with labels
# ----------------------------------------------------------

sub showtriggers
{
foreach $item (Win32::OLE::in ($world->GetTriggerList))
 {
 $world->note($item);
 }
} # end of showtriggers


# -----------------------------------------------
# Example showing iterating through all variables
# ------------------------------------------------

sub showvariables
{
foreach $item (Win32::OLE::in ($world->GetVariableList))
 {
 ($key, $value) = ($item, $world->GetVariable ($item));
 $world->note($key . " = " . $value) if (defined ($key));
 }
} # end of showvariables 


# ----------------------------------------------------------
# Example showing iterating through all aliases with labels
# ----------------------------------------------------------

sub showaliases
{
foreach $item (Win32::OLE::in ($world->GetAliasList))
 {
 $world->note($item);
 }
} # end of showaliases 


# ---------------------------------------------------------
# Example showing running a script on world open
# ---------------------------------------------------------
sub OnWorldOpen
{
$world->note ("---------- World Open ------------");
} # end of OnWorldOpen 

# ---------------------------------------------------------
# Example showing running a script on world close
# ---------------------------------------------------------
sub OnWorldClose
{
$world->note ("---------- World Close ------------");
} # end of OnWorldClose 

# ---------------------------------------------------------
# Example showing running a script on world connect
# ---------------------------------------------------------
sub OnWorldConnect
{
$world->note ("---------- World Connect ------------");
} # end of OnWorldConnect 

# ---------------------------------------------------------
# Example showing running a script on world disconnect
# ---------------------------------------------------------
sub OnWorldDisconnect
{
$world->note ("---------- World Disconnect ------------");
} # end of OnWorldDisconnect 


# ---------------------------------------------------------
# Example showing running a script on an alias
#
#  This script is designed to be called by an alias: ^teleport(.*)$
#
#  This alias SHOULD have "regular expression" checked.
#
#  It is for teleporting (going to) a room by number
#
#  The room is entered by name and looked up in the variables
#  list.
# ---------------------------------------------------------
sub OnTeleport
{

my ($thename, $theoutput, $wildcards) = @_;

$sDestination = $world->trim ($world->GetAliasInfo ($thename, 101));

# if nothing entered echo possible destinations

if ($sDestination eq "") 
  {
  $world->note ("-------- TELEPORT destinations ----------");

  foreach $item (Win32::OLE::in ($world->GetVariableList()))
   {
   ($key, $value) = ($item, $world->GetVariable ($item));
   if (substr ($key, 0, 9) eq "teleport_")
     {
     $sHelp .= ", " if ($sHelp ne "");
     $sHelp .= substr ($key, 9);
     }
   }

  # if no destinations found, tell them
  
  $sHelp = "<no rooms in teleport list>" if ($sHelp eq "");
  $world->note ($sHelp);
  return;
  }    # no destination supplied

# get contents of the destination variable 

$iRoom = $world->GetVariable ("teleport_" . lc ($sDestination));

# if not found, or invalid name, that isn't in the list
if (!defined ($iRoom))
  {
  $world->note ("******** Destination $sDestination unknown *********");
  return;
  }

$world->note ("------> Teleporting to $sDestination");
$world->send ("\@teleport #$iRoom");

}	# end of OnTeleport 

# ---------------------------------------------------------
# Example showing running a script on an alias
#
#  This script is designed to be called by an alias:  ^add_teleport(|\s*(\w*)\s*(\d*))$
#
#  This alias SHOULD have "regular expression" checked.
#
#  It is for adding a room to the list of rooms to teleport to (by
#  the earlier script).
#
#  eg.  ADD_TELEPORT dungeon 1234
#
# ---------------------------------------------------------
sub OnAddTeleport 
{
my ($thename, $theoutput, $wildcards) = @_;

# wildcard 2 is the room name

$sDestination = $world->trim ($world->GetAliasInfo ($thename, 102));

# if nothing entered tell them command syntax
if ($sDestination eq "")
  {
  $world->note ("Syntax: add_teleport name dbref");
  $world->note ("    eg. add_teleport LandingBay 4421");
  return;
  }

# wildcard 3 is where to go to

$iRoom = $world->trim ($world->GetAliasInfo ($thename, 103));

# add room and destination location to variable list
$iStatus = $world->SetVariable ("teleport_$sDestination", $iRoom);

if ($iStatus != 0)
  {
  $world->note ("Room name must be alphabetic, you entered: $sDestination");
  return;
  }

$world->note ("Teleport location $sDestination (#$iRoom) added to teleport list");

}  # end of OnAddTeleport 


# ------------------------------------------
# Example showing a script called by a timer
# -------------------------------------------
sub OnTimer
{
my ($strTimerName) = @_;

$world->note ("Timer $strTimerName has fired!");
}	# end of OnTimer 

# --------------------------------------------
# Example showing a script called by a trigger
#  Should be connected to a trigger matching on: <*hp *m *mv>*
#   (the above example will work for SMAUG default prompts (eg. <100hp 10m 40mv>)
#    it may need to be changed depending on the MUD prompt format).
# --------------------------------------------
sub OnStats
{
my ($strTriggerName, $trig_line, $wildcards) = @_;

$iHP = $world->GetTriggerInfo ($strTriggerName, 101);
$iMana = $world->GetTriggerInfo ($strTriggerName, 102);
$iMV = $world->GetTriggerInfo ($strTriggerName, 103);

$world->Note ("Your HP are $iHP");
$world->Note ("Your Mana is $iMana");
$world->Note ("Your movement points are $iMV");

}	# end of OnStats 



# --------------------------------------------
# Subroutine to be called to repeat a command.
#
#  Call from the alias: ^#(\d+)\s+(.+)$
#  Regular Expression: checked
#
#  Example of use:  #10 give sword to Trispis
#  This would send "give sword to Trispis" 10 times
# --------------------------------------------
sub OnRepeat
{
my ($thename, $theoutput, $wildcards) = @_;

  $iCount   = $world->GetAliasInfo ($thename, 101);  # count of times
  $iCommand = $world->GetAliasInfo ($thename, 102);  # what to send

  for ($i = 1; $i <= $iCount; $i++)
    {
    $world->Send ($iCommand);
    }
}  # end of OnRepeat 



# --------------------------------------------
# Example showing iterating through all worlds
# --------------------------------------------

sub showworlds
{
 foreach $item (Win32::OLE::in ($world->GetWorldList))
   {
   $world->note($item);
   }

}	# end of showworlds 

# --------------------------------------------------
# Example showing sending a message to another world
# --------------------------------------------------

sub SendToWorld 
 {
 my ($name, $message) = @_;

 my $otherworld;

 $otherworld = $world->getworld ($name);

  if (!defined ($otherworld))
    {
    $world->note("World " . $name . " is not open");
    return;
    }

  $otherworld->send($message);

 }

# --------------------------------------------
# Example trigger routine that just shows what was passed to it
# --------------------------------------------
 
sub ExampleTrigger
{
 my ($thename, $theoutput, @$wildcards) = @_;

 $world->note ("Trigger " . $thename . " fired.");
 $world->note ("Matching line was: " . $theoutput);

 for ($i = 1; $i <= 10; $i++)
   {
   $wildcard = $world->GetTriggerInfo ($thename, 100 + $i);
   $world->note ("Wildcard $i = $wildcard");
   }
}


# --------------------------------------------
# Subroutine to be called remember which way you walked.
#
#  Call from the alias: keypad-*
#  Send: %1
#
# --------------------------------------------
sub OnKeypadDirection
{
my ($thename, $theoutput, $wildcards) = @_;

  $Direction = $world->GetAliasInfo ($thename, 101);
  $world->setvariable("direction", $Direction);
}

$world->note ("Scripting enabled - script file processed");

