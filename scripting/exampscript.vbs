' Example of coding script routines for MUSHclient in VBscript

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
const eArrayAlreadyExists = 30055 ' That array already exists
const eBadKeyName = 30056 ' That name is not permitted for a key
const eArrayDoesNotExist = 30056 ' That array does not exist
const eArrayNotEvenNumberOfValues = 30057 ' Values to be imported into array are not in pairs
const eImportedWithDuplicates = 30058 ' Import succeeded, however some values were overwritten
const eBadDelimiter = 30059 ' Import/export delimiter must be a single character, other than backslash
const eSetReplacingExistingValue = 30060 ' Array element set, existing value overwritten
const eKeyDoesNotExist = 30061 ' Array key does not exist
const eCannotImport = 30062 ' Cannot import because cannot find unused temporary character


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
const eTemporary = 16384 ' temporary - do not save to world file 

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

' ----------------------------------------------------------
' Example showing iterating through all triggers with labels
' ----------------------------------------------------------
sub showtriggers 

dim mylist
dim i

mylist = world.GetTriggerList

if not IsEmpty (mylist) then
  for i = lbound (mylist) to ubound (mylist)
    world.note mylist (i)
  next
End If

end sub

' -----------------------------------------------
' Example showing iterating through all variables
' ------------------------------------------------
sub showvariables 

dim mylist
dim i

mylist = world.GetVariableList

if not IsEmpty (mylist) then
  for i = lbound (mylist) to ubound (mylist)
    world.note mylist (i) & " = " & world.GetVariable (mylist (i))
  next
End If

end sub

' ---------------------------------------------------------
' Example showing iterating through all aliases with labels
' ---------------------------------------------------------
sub showaliases

dim mylist
dim i

mylist = world.GetAliasList

if not IsEmpty (mylist) then
  for i = lbound (mylist) to ubound (mylist)
    world.note mylist (i)
  next
End If

end sub

' ---------------------------------------------------------
' Example showing running a script on world open
' ---------------------------------------------------------
sub OnWorldOpen
world.note "---------- World Open ------------"
end sub

' ---------------------------------------------------------
' Example showing running a script on world close
' ---------------------------------------------------------
sub OnWorldClose
world.note "---------- World Close ------------"
end sub

' ---------------------------------------------------------
' Example showing running a script on world connect
' ---------------------------------------------------------
sub OnWorldConnect
world.note "---------- World Connect ------------"
end sub

' ---------------------------------------------------------
' Example showing running a script on world disconnect
' ---------------------------------------------------------
sub OnWorldDisconnect
world.note "---------- World Disconnect  ------------"
end sub

' ---------------------------------------------------------
' Example showing running a script on an alias
'
'  This script is designed to be called by an alias: ^teleport(.*)$
'
'  This alias should have "regular expression" checked.
'
'  It is for teleporting (going to) a room by number
'
'  The room is entered by name and looked up in the variables
'  list.
' ---------------------------------------------------------
sub OnTeleport (thename, theoutput, thewildcards)

dim sDestination
dim sRoomList
dim sHelp
dim iSubscript
dim iRoom

sDestination = Trim (thewildcards (1))

' if nothing entered echo possible destinations
if sDestination = "" then
  world.note "-------- TELEPORT destinations ----------"	

  ' find list of all variables
  sRoomList = world.GetVariableList

  if not IsEmpty (sRoomList) then

    ' loop through each variable, and add to help if it starts with "teleport_"
    for iSubscript = lbound (sRoomList) to ubound (sRoomList)
       if Left (sRoomList (iSubscript), 9) = "teleport_" then
          if sHelp <> "" then
             sHelp = sHelp & ", "
          end if
          sHelp = sHelp & Mid (sRoomList (iSubscript), 10)
       end if   ' variable starts with "teleport_"
    next        ' loop through sRoomList

  end if	' having at least one room

  ' if no destinations found, tell them
  if sHelp = "" then
    sHelp = "<no rooms in teleport list>"
  end if    ' no destinations found in list
  world.note sHelp
  exit sub

end if    ' no destination supplied

' get contents of the destination variable 
iRoom = world.GetVariable ("teleport_" & lCase (sDestination))

' if not found, or invalid name, that isn't in the list
if IsEmpty (iRoom) or IsNull (iRoom) then
  world.note "******** Destination " & sDestination & " unknown *********"
  exit sub
end if

world.note "------> Teleporting to " & sDestination
world.send "@teleport #" & cstr (iRoom)

end sub

' ---------------------------------------------------------
' Example showing running a script on an alias
'
'  This script is designed to be called by an alias:  ADD_TELEPORT  * *
'
'  This alias should NOT have "regular expression" checked.
'
'  It is for adding a room to the list of rooms to teleport to (by
'  the earlier script).
'
'  eg.  ADD_TELEPORT  dungeon 1234
'
' ---------------------------------------------------------
sub OnAddTeleport (thename, theoutput, thewildcards)

dim sDestination
dim iRoom
dim iStatus
dim sCurrentLocation

' wildcard one is the destination
sDestination = Trim (thewildcards (1))

' if nothing entered tell them command syntax
if sDestination = "" then
  world.note "Syntax: add_teleport name dbref"
  world.note "    eg. add_teleport LandingBay 4421"
  exit sub
end if

' wildcard 2 is the room to go to
iRoom= Trim (thewildcards (2))

if not IsNumeric (iRoom) then
  world.note "Room to teleport to must be a number, you entered: " & iRoom
  exit sub
end if

' add room and destination location to variable list
iStatus = world.SetVariable ("teleport_" & sDestination, iRoom)

if iStatus <> 0 then
  world.note "Room name must be alphabetic, you entered: " & sDestination
  exit sub
end if

world.note "Teleport location " & sDestination & "(#" _
           & iRoom & ") added to teleport list"

end sub


' ------------------------------------------
' Example showing a script called by a timer
' -------------------------------------------
sub OnTimer (strTimerName)
world.note "Timer has fired!"
end sub


' --------------------------------------------
' Example showing a script called by a trigger
'  Should be connected to a trigger matching on: <*hp *m *mv>*
'   (the above example will work for SMAUG default prompts (eg. <100hp 10m 40mv>)
'    it may need to be changed depending on the MUD prompt format).
' --------------------------------------------
sub OnStats (strTriggerName, trig_line, arrWildCards)

dim iHP
dim iMana
dim iMV

iHP = arrWildCards (1)
iMana = arrWildCards (2)
iMV = arrWildCards (3)

world.Note "Your HP are " & iHP
world.Note "Your Mana is " & iMana
world.Note "Your movement points are " & iMV

end sub

' --------------------------------------------
' Example showing an alias used to test lag
'  Should be connected to an alias matching on: lag
' --------------------------------------------
sub OnLagTest (strAliasName, strOutput, arrWildCards)
  World.Send "@pem/silent me=PING: " & now()
end sub

' --------------------------------------------
' Example showing a trigger used as part of the lag test
'  Should be connected to a trigger matching on: PING: *
'  Set trigger to: Omit from log file, Omit from output
' --------------------------------------------
Sub OnPing (strTriggerName, strOutput, arrWildCards)
Dim CurrentTime, PingTime, Elapsed
  CurrentTime = Now ()
  PingTime = arrWildCards (1)
  Elapsed = DateDiff ("s", PingTime, CurrentTime)
  World.SetStatus "Lag: " & Elapsed & " seconds."
End Sub

' ---------------------------------------------------------
' Example showing running a script on world connect
'  This script turns logging on for this world.
' ---------------------------------------------------------
Sub OnWorldConnectWithLogging
Dim FileName
  
  FileName = Replace (World.Worldname, " ", "") ' Remove spaces from file name

' Add other editing here if you have weird characters in your mud names.

  FileName = FileName & ".txt"
  if World.OpenLog (FileName, True) = eOK then   ' true means append to any existing log file
    World.WriteLog "--- Log Opened " & Now () & " ---"  ' make note in log file
    World.Note "Opened log file" & FileName   ' make note in world window
  end if
End Sub

' ---------------------------------------------------------
' Example showing running a script on world disconnect
'  This script turns logging off for this world.
' ---------------------------------------------------------
Sub OnWorldDisconnectWithLogging

  if World.IsLogOpen then
     World.Note "Closing log file"   ' note in world window we are closing the log
     World.WriteLog "--- Log Closed " & Now () & " ---"  ' note in log file
     World.CloseLog  ' close log file
  end if

End Sub

' --------------------------------------------
' Subroutine to be called to repeat a command.
'
'  Call from the alias: ^#([0-9]+) (.+)$
'  Regular Expression: checked
'
'  Example of use:  #10 give sword to Trispis
'  This would send "give sword to Trispis" 10 times
' --------------------------------------------
Sub OnRepeat (strAliasName, strOutput, arrWildCards)
Dim i
  For i = 1 to arrWildCards (1)
   World.Send arrWildCards (2)
  next
End Sub


' --------------------------------------------
' Subroutine to be called to gag a player
'
' Call from the alias: gag *
' Regular expression: no
' Label: lblGag
' Script: OnGag
'
' Example of use: gag twink
'
' This updates a trigger "lblGag"
' --------------------------------------------
Sub OnGag (strAliasName, strOutput, arrWildCards)

Dim trMatch
Dim trResponse
Dim trFlags
Dim trColour
Dim trWildcard
Dim trSoundfilename
Dim trScriptname
Dim iStatus

' If trigger already exists, see who it matches on, then delete it
If world.IsTrigger ("lblGag") = eOK Then
  world.GetTrigger "lblGag", trMatch, trResponse, trFlags, _
		   trColour, trWildcard, trSoundfilename, trScriptname
  world.DeleteTrigger "lblGag"
  trMatch = trMatch & "|"  ' Separate names by | ("or" symbol)
End If

' Add new person to list of names to match on
trMatch = trMatch & LCase (Trim (arrWildCards (1)))	' add new name to list

' Add our new trigger to trigger list
world.AddTrigger "lblGag", trMatch, "", _
                 eEnabled + eOmitFromLog + eOmitFromOutput + eIgnoreCase + eTriggerRegularExpression, _
		NOCHANGE, 0, "", ""

' Tell the user what we did
world.Note "Added " & arrWildcards (1) & " to list of gags"

End Sub

' --------------------------------------------
' Example of copying a file in a subroutine
'
' This might be used on a "connect" to change your web page to indicate
' you are connected to a MUD, then use a similar one to copy a different
' file to indicate you are disconnected.
' --------------------------------------------
Sub CopyFile
Const ForReading = 1
Const ForWriting = 2

  Dim fso, tf, contents

  Set fso = CreateObject("Scripting.FileSystemObject")

  Set tf = fso.OpenTextFile("c:\file_connected.htm", ForReading)
  contents = tf.ReadAll
  tf.Close

  Set tf = fso.OpenTextFile("c:\index.htm ", ForWriting, True)
  tf.Write (contents)
  tf.Close

End Sub

' --------------------------------------------
' Example showing iterating through all worlds
' --------------------------------------------
sub ShowWorlds 

  dim mylist
  dim i

  mylist = world.GetWorldList

  if not IsEmpty (mylist) then
    for i = lbound (mylist) to ubound (mylist)
      world.note mylist (i)
  next
  End If

end sub

' --------------------------------------------------
' Example showing sending a message to another world
' --------------------------------------------------

sub SendToWorld (name, message)
dim otherworld

  set otherworld = world.getworld (name)

  if otherworld is nothing then
    world.note "World " + name + " is not open"
    exit sub
  end if

  otherworld.send message

end sub

' --------------------------------------------
' Subroutine to be called remember which way you walked.
'
'  Call from the alias: keypad-*
'  Send: %1
'
' --------------------------------------------
Sub OnKeypadDirection (strAliasName, strOutput, arrWildCards)
  world.setvariable "direction", arrWildcards (1)
End Sub

' --------------------------------------------
' Trigger to count experience points.
' Match on: You are awarded * experience points for the battle.
' --------------------------------------------

dim gTotalExperience	' this is a global variable
gTotalExperience = 0    ' initialise it

Sub OnExperience (strTriggerName, strOutput, arrWildCards)
  gTotalExperience = gTotalExperience + arrWildCards (1)
  World.SetStatus "Experience earned is now: " & CStr (gTotalExperience) 
End Sub

' --------------------------------------------
' Subroutine to discard the speedwalk queue.
'
'  Call from the alias: DISCARD
'
' --------------------------------------------
Sub OnDiscardQueue (strAliasName, strOutput, arrWildCards)
  world.discardqueue
End Sub

' --------------------------------------------
' Subroutine to replace one name with another (Name1 with Name2)
'
'  Call from the trigger: 
'
'  Match on: Name1
'  Send: <nothing>
'  Regular Expression: checked
'  Ignore case: checked
'  Omit from output: checked
'  Omit from log: checked
'  Label:  NameChange
'  Script: OnNameChange
'
' --------------------------------------------
sub OnNameChange (strTriggerName, trig_line, arrWildCards)
dim strChangedLine
 
  strChangedLine = world.Replace (trig_line, "Name1", "Name2", true)
  strChangedLine = world.Replace (strChangedLine, "name1", "Name2", true)
  world.note strChangedLine
  world.writelog  strChangedLine  

End Sub

' --------------------------------------------
' Alias to gag players automatically
'
'  Call from the alias: GAG *
'
'   eg. GAG twerp
'
'  It maintains a list of gagged players in the
'   variable "gaglist" (which you can edit in the
'   world configuration screen if you need)
'
'  You may need to change the text of the trigger
'  match slightly. (eg. add "tells")
'
' --------------------------------------------

sub OnGag (thename, theoutput, thewildcards)
dim sName
dim sList
dim flags

sName = Trim (thewildcards (1))

' do nothing if no name
if sName = "" then exit sub

' get existing list
sList = world.GetVariable ("gaglist")

' if empty create new one, otherwise add to end
if IsEmpty (sList) then
  sList = sName
else
  slist = sList + "|" + sName
end if

' remember gag list for next time
world.SetVariable "gaglist", sList

' addtrigger flags
'    1 = enabled
'    2 = omit from log file
'    4 = omit from output
'   16 = ignore case
'   32 = regular expression
' 1024 = replace existing trigger of same name

flags = 1 + 2 + 4 + 16 + 32 + 1024

World.addtrigger "gag", "^(" + sList + ") (says|pages)", _
                 "", flags, -1, 0, "", ""

End Sub

' --------------------------------------------
'  ShowOptions
'
'  Lists options set in MUSHclient. 
'
'  Type: /ShowOptions
'
'  To change an option do something like this:
'
'   /world.setoption "max_output_lines", 2000
'   /world.setoption "mud_can_remove_underline", 1
'
' --------------------------------------------

sub ShowOptions
dim mylist
  dim i

  mylist = world.GetOptionList

  if not IsEmpty (mylist) then
    for i = lbound (mylist) to ubound (mylist)
      world.note mylist (i) + " = " + cstr (world.GetOption ( mylist (i)))
    next
  End If
End Sub

' --------------------------------------------
'  MXP callbacks
' --------------------------------------------

' --------------------------------------------
'  MXP startup
'
'  Called when MXP is activated.
' --------------------------------------------

sub OnMXPStartUp
  world.note "MXP started up"
end sub

' --------------------------------------------
'  MXP shutdown
'
'  Called when MXP is deactivated.
' --------------------------------------------

sub OnMXPShutDown
  world.note "MXP shut down"
end sub


' --------------------------------------------
'  Errors/warnings/information/messages
'
'  Called by MXP when there is an error or warning etc.
'
'  Level is:
'   E - error
'   W - warning
'   I - information
'   A - all other
'
'  Number is the error number (see forum for details)
'
'  Line is the line number in the output window
'
'  Message is the text of the message
'
'  Return true to suppress the message from appearing in
'  the MXP debug window, false to allow it to appear.
' --------------------------------------------

function OnMXPError (level, number, line, message)

  ' display warnings and errors in the output window
  if level = "W" or level = "E" then 
    world.NoteColourName "white", "red"
    world.Note level + "(" + cstr (number) + ")[ " + cstr(line) + "]: " + message
  end if

 ' suppress error number 20001 from the debug window
 if number = 20001 then OnMXPError = true

end function

' --------------------------------------------
' MXP start tag
'
'  Called when a server-defined, or built-in tag is encountered
'   (eg. <color fore=red back=blue> )
'
'  Name is tag name (lowercase), eg. "font"
'
'  Args is the argument list (eg. "fore=red back=blue")
'
'  Mylist is the argument list, parsed into an array, eg.
'   Array item 0 : fore=red
'   Array item 1 : back=blue
'
'  Unnamed arguments are preceded by an argument number.
'    eg. <color red blue> would give:
'
'   Array item 0 : 1=red
'   Array item 1 : 2=blue
'
'  
'  Return true to suppress the tag from having an effect
'  otherwise return false to allow it to act.
' --------------------------------------------
function OnMXPStartTag (name, args, mylist)

  dim i

  world.AppendToNotepad "MXP", "Opening tag: " + name + vbCRLF

  if not IsEmpty (mylist) then
    for i = lbound (mylist) to ubound (mylist)
      world.AppendToNotepad "MXP", "Arg " + cstr (i) + " = " + mylist (i) + vbCRLF
  next
  End If

  ' ignore tag <rdesc>
  if name = "rdesc" then OnMXPStartTag = true

end function

' --------------------------------------------
' MXP set variable
'
'  This is called when MXP sets a variable.
'
'  Name - variable name (always preceded by "mxp_")
'  Contents - what it is being set to.
' --------------------------------------------
sub OnMXPvariable (name, contents)
  world.note "Var " + name + " set to " + contents
end sub

' --------------------------------------------
' MXP end tag
'
'  This is called when an MXP end tag is processed
'    (eg. </color>
'
'  Name - tag name (lowercase) (eg. "color")
'  Text - text between start and end tag
'         eg. <send>Go West</send>
'    When the </send> is received Text will be "Go West"
'
' --------------------------------------------
sub OnMXPEndTag (name, text)
  world.AppendToNotepad "MXP", "Closing tag: " + name + vbCRLF
end sub

' ---------------------------------------------------------
'
'  See if memberName is in listName
'
'  eg.  if IsMember ("spell_list", "fiery blast") then
'          ... do something ...
'
'
' ---------------------------------------------------------
Function IsMember (listName, memberName)
dim ListContents
dim theList
dim WantedMember
dim i

' Default to not found
IsMember = False

' Fix up member to remove surrounding spaces and make lower case
WantedMember = Trim (LCase (memberName))

' Can't find blank items
If WantedMember = "" Then
  Exit Function
End If

' Get list
ListContents = World.GetVariable (listName)

' If no list, the member can't be in it
If IsEmpty (ListContents) or IsNull (ListContents) then
  Exit Function
End If

' Split list at commas
theList = split (ListContents, ",")

' Loop through list, seeing if wanted member is in it
If Not IsEmpty (theList) then
  For i = lbound (theList) to ubound (theList)
  If theList (i) = WantedMember then
    IsMember = True
    Exit Function
  End If      ' end of found it
  Next        ' end of loop
End If        ' end of any items in list

End Function

' ---------------------------------------------------------
'
'  Add memberName to listName
'
'  eg.  AddItem  "spell_list", "fiery blast"
'
'  Note - it is up to you to test if the item is already
'         in the list (using IsMember) or it will be added 
'         twice.
'
'
' ---------------------------------------------------------
Sub AddItem (listName, memberName)
dim ListContents
dim theList
dim NewMember
dim i

' Get existing list
ListContents = World.GetVariable (listName)

' Make sure new member is lower case with no spaces around it
NewMember = Trim (LCase (memberName))

' Don't add blank items
If NewMember = "" Then
   Exit Sub
End If

' Need comma after previous member, if any
If ListContents <> "" Then
  ListContents = ListContents & ","
End If
 
' Add new item
ListContents = ListContents & NewMember

World.SetVariable listName, ListContents

End Sub

' ---------------------------------------------------------
'
'  Delete memberName from listName
'
'  eg. DeleteItem "spell_list", "fiery blast"
'
'
' ---------------------------------------------------------
Sub DeleteItem (listName, memberName)
dim ListContents
dim theList
dim WantedMember
dim i
dim NewContents

' Fix up member to remove surrounding spaces and make lower case
WantedMember = Trim (LCase (memberName))

' Can't delete blank items
If WantedMember = "" Then
  Exit Sub
End If

' Get list
ListContents = World.GetVariable (listName)

' If no list, the member can't be in it
If IsEmpty (ListContents) or IsNull (ListContents) then
  Exit Sub
End If

' Split list at commas
theList = split (ListContents, ",")

' New contents are empty now
NewContents = ""

' Loop through list, copying across all but delete item
If Not IsEmpty (theList) then
  For i = lbound (theList) to ubound (theList)
  If theList (i) <> WantedMember then
    If NewContents <> "" Then
       NewContents = NewContents & ","
    End If
  NewContents = NewContents & theList (i)
  End If      ' end of not deleted item
  Next        ' end of loop
End If        ' end of any items in list

' Store new list contents
World.SetVariable listName, NewContents

End Sub


' ---------------------------------------------------------
'
'  Delete the entire list called listName
'
'  eg. DeleteList "spell_list"
'
'
' ---------------------------------------------------------
Sub DeleteList (listName)

World.DeleteVariable listName

End Sub

' ---------------------------------------------------------
' Example of opening another world from an alias.
'
' Alias: openworld *
' Label: OpenWorld
' Script: OpenWorld
' ---------------------------------------------------------
Sub OpenWorld (thename, theoutput, thewildcards)
dim otherworld
dim filename

  filename = "worlds\" & thewildcards (1) & ".mcl"
  set otherworld = world.open  (filename)

  if otherworld is nothing then
    world.note "Could not open world file " & filename
  else
    otherworld.activate
  end if
  
End Sub

' ---------------------------------------------------------
' Example of toggling from one world to the next
'
' Alias: toggle
' Label: ToggleWorld
' Script: ToggleWorld
' ---------------------------------------------------------
Sub ToggleWorld (thename, theoutput, thewildcards)
  dim mylist
  dim i
  dim thisworld
  dim otherworld

  thisworld = 0

  ' get list of worlds
  mylist = world.GetWorldList

  ' find which world we are
  If not IsEmpty (mylist) then
    for i = lbound (mylist) to ubound (mylist)
      If world.WorldName = mylist (i) then
         thisworld = i
      End If
  next
  End If
 
  ' find next world, wrap around at end of list
  if thisworld = ubound (mylist) then
     thisworld = 0
  else
     thisworld = thisworld + 1
  End If

  ' get reference to next world
  set otherworld = world.GetWorld (mylist (thisworld))

  ' activate it
  If not (otherworld is nothing) then
    otherworld.activate
  End If

End Sub

' ---------------------------------------------------------
' Example of going to another world
'
' Alias: world *
' Label: GoToWorld
' Script: GoToWorld
' ---------------------------------------------------------
Sub GoToWorld (thename, theoutput, thewildcards)
  dim mylist
  dim i
  dim otherworld

  ' get list of worlds
  mylist = world.GetWorldList

  i = cint (thewildcards (1))

  if i < 0 or i > ubound (mylist) then
     world.note "You do not have a world number " & i
     exit sub
  End If

  ' get reference to next world
  set otherworld = world.GetWorld (mylist (i))

  ' activate it
  If not (otherworld is nothing) then
    otherworld.activate
  End If

End Sub

' ---------------------------------------------------------
' Alias to enable all triggers
'
' Alias: enable_triggers
' Label: EnableTriggers 
' Script: EnableTriggers 
' ---------------------------------------------------------

Sub EnableTriggers (thename, theoutput, thewildcards)
world.setoption "enable_triggers", 1
End Sub

' ---------------------------------------------------------
' Alias to disable all triggers
'
' Alias: disable_triggers
' Label: DisableTriggers 
' Script: DisableTriggers 
' ---------------------------------------------------------

Sub DisableTriggers (thename, theoutput, thewildcards)
world.setoption "enable_triggers", 0
End Sub

world.note "Scripting enabled - script file processed"

