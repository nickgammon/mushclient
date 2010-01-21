// flags.h - flags for adding triggers, aliases, timers and so on


// flags for AddTrigger
enum
  {
  eEnabled = 0x01,
  eOmitFromLog = 0x02,
  eOmitFromOutput = 0x04,
  eKeepEvaluating = 0x08,
  eIgnoreCase = 0x10,
  eTriggerRegularExpression = 0x20,
  eExpandVariables = 0x200,
  eReplace = 0x400,
  eLowercaseWildcard = 0x800,
  eTemporary = 0x4000,
  eTriggerOneShot = 0x8000,
  };

// flags for AddAlias
enum
  {
//  eEnabled = 0x01,    // same as above
  eUseClipboard = 0x02,
//  eDelayed = 0x04,    // delayed send
//  eMatchStart = 0x04,
//  eMatchAnywhere = 0x08,
//  eMatchExactly = 0x10,
  eIgnoreAliasCase = 0x20,
  eOmitFromLogFile = 0x40,
  eAliasRegularExpression = 0x80,
  eAliasOmitFromOutput = 0x100,
//  eExpandVariables = 0x200,
//  eReplace = 0x400,   // same as above
  eAliasSpeedWalk = 0x800,
  eAliasQueue = 0x1000,
  eAliasMenu = 0x2000,
//  eTemporary = 0x4000,  // same as above
  eAliasOneShot = 0x8000,

  };

// flags for AddTimer
enum
  {
//  eEnabled = 0x01,    // same as above
  eAtTime = 0x02,   // if not set, time is "every"
  eOneShot = 0x04,
  eTimerSpeedWalk = 0x08,   // timer does speed walk
  eTimerNote = 0x10,        // timer does note
  eActiveWhenClosed = 0x20,  // timer fires when world closed
//  eReplace = 0x400,  // same as above
//  eTemporary = 0x4000,  // same as above
  };
