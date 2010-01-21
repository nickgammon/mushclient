// Example of coding script routines for MUSHclient in Jscript

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
var eArrayAlreadyExists = 30055; // That array already exists
var eBadKeyName = 30056; // That name is not permitted for a key
var eArrayDoesNotExist = 30056; // That array does not exist
var eArrayNotEvenNumberOfValues = 30057; // Values to be imported into array are not in pairs
var eImportedWithDuplicates = 30058; // Import succeeded, however some values were overwritten
var eBadDelimiter = 30059; // Import/export delimiter must be a single character, other than backslash
var eSetReplacingExistingValue = 30060; // Array element set, existing value overwritten
var eKeyDoesNotExist = 30061; // Array key does not exist
var eCannotImport = 30062; // Cannot import because cannot find unused temporary character


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
var eTemporary = 16384; // temporary - do not save to world file 


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


// ----------------------------------------------------------
// Example showing iterating through all triggers with labels
// ----------------------------------------------------------

function showtriggers ()
{
triggerlist = new VBArray(world.GetTriggerList()).toArray();

if (triggerlist)  // if not empty
 for (i = 0; i < triggerlist.length; i++)
   world.note(triggerlist [i]);
}	// end of showtriggers


// -----------------------------------------------
// Example showing iterating through all variables
// ------------------------------------------------

function showvariables ()
{
variablelist = new VBArray(world.GetVariableList()).toArray();

if (variablelist)  // if not empty
 for (i = 0; i < variablelist.length; i++)
   world.note(variablelist [i] + " = " + world.GetVariable(variablelist [i]));
}	// end of showvariables 


// ----------------------------------------------------------
// Example showing iterating through all aliases with labels
// ----------------------------------------------------------

function showaliases ()
{
aliaslist = new VBArray(world.GetAliasList()).toArray();

if (aliaslist)  // if not empty
 for (i = 0; i < aliaslist.length; i++)
   world.note(aliaslist [i]);
}	// end of showaliases 


// ---------------------------------------------------------
// Example showing running a script on world open
// ---------------------------------------------------------
function OnWorldOpen ()
{
world.note ("---------- World Open ------------");
}	// end of OnWorldOpen 

// ---------------------------------------------------------
// Example showing running a script on world close
// ---------------------------------------------------------
function OnWorldClose ()
{
world.note ("---------- World Close ------------");
}	// end of OnWorldClose 

// ---------------------------------------------------------
// Example showing running a script on world connect
// ---------------------------------------------------------
function OnWorldConnect ()
{
world.note ("---------- World Connect ------------");
}	// end of OnWorldConnect 

// ---------------------------------------------------------
// Example showing running a script on world disconnect
// ---------------------------------------------------------
function OnWorldDisconnect ()
{
world.note ("---------- World Disconnect  ------------");
}	// end of OnWorldDisconnect 

// ---------------------------------------------------------
// Example showing running a script on an alias
//
//  This script is designed to be called by an alias: ^teleport(.*)$
//
//  This alias SHOULD have "regular expression" checked.
//
//  It is for teleporting (going to) a room by number
//
//  The room is entered by name and looked up in the variables
//  list.
// ---------------------------------------------------------
function OnTeleport (thename, theoutput, wildcardsVB)
{

var sDestination;
var sHelp = "";
var iSubscript;
var iRoom;
var mylistsa;		// safe array
var mylistvba;		// vb array
var sRoomList;      // jscript array

wildcards = VBArray(wildcardsVB).toArray();	

sDestination = wildcards [0];

// remove leading spaces from destination
while (sDestination.substr (0, 1) == " ")
  sDestination = sDestination.substr (1);

// if nothing entered echo possible destinations
if (sDestination == "") 
  {
  world.note ("-------- TELEPORT destinations ----------");

  mylistsa = world.GetVariableList ();

  // find list of all variables
  if (mylistsa != null)
    {

    mylistvba = new VBArray(mylistsa);
    sRoomList = mylistvba.toArray ();

    // loop through each variable, and add to help if it starts with "teleport_"
    for (iSubscript = mylistvba.lbound (); iSubscript <= mylistvba.ubound (); iSubscript++)
       {
       if (sRoomList [iSubscript].substr (0, 9) == "teleport_")
          {
          if (sHelp != "")
             sHelp += ", ";
          sHelp = sHelp + sRoomList [iSubscript].substr (9);
          } // variable starts with "teleport_"
       }        // loop through sRoomList

    } // having at least one room

  // if no destinations found, tell them
  if (sHelp == "")
    sHelp = "<no rooms in teleport list>"
  world.note (sHelp);
  return;
  }    // no destination supplied

// get contents of the destination variable 

iRoom = world.GetVariable ("teleport_" + sDestination.toLowerCase ());

// if not found, or invalid name, that isn't in the list
if (iRoom == null)
  {
  world.note ("******** Destination " + sDestination + " unknown *********");
  return;
  }

world.note ("------> Teleporting to " + sDestination);
world.send ("@teleport #" + iRoom);

}	// end of OnTeleport 


// ---------------------------------------------------------
// Example showing running a script on an alias
//
//  This script is designed to be called by an alias:  ^add_teleport(|\s*(\w*)\s*(\d*))$
//
//  This alias SHOULD have "regular expression" checked.
//
//  It is for adding a room to the list of rooms to teleport to (by
//  the earlier script).
//
//  eg.  ADD_TELEPORT dungeon 1234
//
// ---------------------------------------------------------
function OnAddTeleport (thename, theoutput, wildcardsVB)
{
var sDestination
var iRoom
var iStatus
var sCurrentLocation

wildcards = VBArray(wildcardsVB).toArray();	

// wildcard one is the destination
sDestination = wildcards [1]

// remove leading spaces from destination
while (sDestination.substr (0, 1) == " ")
  sDestination = sDestination.substr (1);

// if nothing entered tell them command syntax
if (sDestination == "")
 {
  world.note ("Syntax: add_teleport name dbref");
  world.note ("    eg. add_teleport LandingBay 4421");
  return;
  }


// wildcard 2 is the room to go to
iRoom= wildcards [2]

// remove leading spaces from destination
while (iRoom.substr (0, 1) == " ")
  iRoom = iRoom.substr (1);

// add room and destination location to variable list
iStatus = world.SetVariable ("teleport_" + sDestination, iRoom);

if (iStatus != 0)
  {
  world.note ("Room name must be alphabetic, you entered: " + sDestination);
  return;
  }

world.note ("Teleport location " + sDestination + "(#" 
           + iRoom + ") added to teleport list");

}  // end of OnAddTeleport 

// ------------------------------------------
// Example showing a script called by a timer
// -------------------------------------------
function OnTimer (strTimerName)
{
world.note ("Timer has fired!");
}	// end of OnTimer 


// --------------------------------------------
// Example showing a script called by a trigger
//  Should be connected to a trigger matching on: <*hp *m *mv>*
//   (the above example will work for SMAUG default prompts (eg. <100hp 10m 40mv>)
//    it may need to be changed depending on the MUD prompt format).
// --------------------------------------------
function OnStats (strTriggerName, trig_line, wildcardsVB)
{

wildcards = VBArray(wildcardsVB).toArray();	

iHP = wildcards [0]
iMana = wildcards [1]
iMV = wildcards [2]

world.Note ("Your HP are " + iHP);
world.Note ("Your Mana is " + iMana);
world.Note ("Your movement points are " + iMV);

}	// end of OnStats 



// --------------------------------------------
// Subroutine to be called to repeat a command.
//
//  Call from the alias: ^#(\d+)\s+(.+)$
//  Regular Expression: checked
//
//  Example of use:  #10 give sword to Trispis
//  This would send "give sword to Trispis" 10 times
// --------------------------------------------
function OnRepeat (strAliasName, strOutput, wildcardsVB)
{
  wildcards = VBArray(wildcardsVB).toArray();	

  for (i = 1; i <= wildcards [0]; i++)
    world.Send (wildcards [1]);
}  // end of OnRepeat 

// --------------------------------------------
// Example showing iterating through all worlds
// --------------------------------------------

function showworlds ()
{
worldlist = new VBArray(world.GetworldList()).toArray();

if (worldlist)  // if not empty
 for (i = 0; i < worldlist.length; i++)
   world.note(worldlist [i]);

}	// end of showworlds 

// --------------------------------------------------
// Example showing sending a message to another world
// --------------------------------------------------

function SendToWorld (name, message)
{
var otherworld

  otherworld = world.getworld (name);

  if (otherworld == null)
    {
    world.note ("World " + name + " is not open");
    return;
    }

  otherworld.send (message);

}

// --------------------------------------------
// Example trigger routine that just shows what was passed to it
// --------------------------------------------

function ExampleTrigger (thename, theoutput, wildcardsVB)
{
  wildcards = VBArray(wildcardsVB).toArray();	

  world.note ("Trigger " + thename + " fired.");
  world.note ("Matching line was: " + theoutput);

  for (i = 0; i < 10; i++)
    if (wildcards [i] != "")
       world.note ("Wildcard " + i + " = " + wildcards [i]);

}

/****************
WEAPON COMPARISON
Compares two weapons and returns the result.

Alias (MUST be regular expression):
	^compare (.+)\s(\d+d\d+)\s(\d+)/(\d+)\s(.+)\s(\d+d\d+)\s(\d+)/(\d+)(|.+)$

Syntax:
	compare <name> <x>d<y> <hr>/<dr> <name> <x>d<y> <hr>/<dr> [action]
	 
Example:
	--> "compare dagger 2d2 4/2 sword 4d6 5/5"
	would compare the dagger to the sword and report which one's the best
	
	you can also include an action...
	
	--> "compare mace 6d7 2/3 pickaxe 2d12 1/4 say"
	would say the best weapon
	
	NOTE: action can be multiple words (e.g "tell vryce")

	
****************/
function CompareWeapons(alias_name, alias_output, wildcardsVB) {
	//Collect wildcards into a Javascript array
	wildcards = VBArray(wildcardsVB).toArray();	

	//General variables
	var sendmud = AllToUpper(wildcards[8].replace(/^\W+/,'')).replace(/\W+$/,'');

	//Weapon one variables
	var name1 = AllToUpper(wildcards[0]);
	var num1 = parseInt(wildcards[1].substr(0,1));
	var dice1 = parseInt(wildcards[1].substr(2));
	var hit1 = parseInt(wildcards[2]);
	var dam1 = parseInt(wildcards[3]);
	var max1 = (num1*dice1)+dam1;
	var min1 = num1+dam1;
	var avg1 = (max1+min1)/2;
	var score1 = avg1+(hit1/2);
	var weap1stat = name1 + ": " + num1 + "d" + dice1 + " hr(" + hit1 + ") dr(" + dam1 + ") min(" + min1 + ") max(" + max1 + ") avg(" + avg1 + ")";
	
	//Weapon two variables
	var name2 = AllToUpper(wildcards[4]);
	var num2 = parseInt(wildcards[5].substr(0,1));
	var dice2 = parseInt(wildcards[5].substr(2));
	var hit2 = parseInt(wildcards[6]);
	var dam2 = parseInt(wildcards[7]);
	var max2 = (num2*dice2)+dam2;
	var min2 = num2+dam2;
	var avg2 = (max2+min2)/2;
	var score2 = avg2+(hit2/2);
	var weap2stat = name2 + ": " + num2 + "d" + dice2 + " hr(" + hit2 + ") dr(" + dam2 + ") min(" + min2 + ") max(" + max2 + ") avg(" + avg2 + ")";
	
		
	//Calculate the better weapon
	if (score1 > score2) {
		var resstring = name1 + " is " + Math.round(100-score2*100/score1) + "% better than " + name2;
	} else if (score1 < score2) {
		var resstring = name2 + " is " + Math.round(100-score1*100/score2) + "% better than " + name1;
	}
	
	//Send output either locally or with specified prefix to mud
	if (sendmud != "") {
		world.send(sendmud + " " + weap1stat);
		world.send(sendmud + " " + weap2stat);
		world.send(sendmud + " " + resstring);
	} else {
		world.note(weap1stat);
		world.note(weap2stat);
		world.note(resstring);
	}
}
/**********
ALLTOUPPER
Returns string with the first letter in each word uppercase
***********/
function AllToUpper(str) 
   {
	aString = (str.replace(/^\W+/,'')).replace(/\W+$/,'').split(" ");
	str = "";
	for (i = 0; i < aString.length; i++) {
		str += aString[i].substr(0,1).toUpperCase() + aString[i].substr(1).toLowerCase() + " ";
	}	
	return str.substr(0,str.length-1);
}

// --------------------------------------------
// Subroutine to be called remember which way you walked.
//
//  Call from the alias: keypad-*
//  Send: %1
//
// --------------------------------------------
function OnKeypadDirection (strAliasName, strOutput, wildcardsVB)
{
  wildcards = VBArray(wildcardsVB).toArray();	
  world.setvariable("direction", wildcards [0]);
}

world.note ("Scripting enabled - script file processed");

