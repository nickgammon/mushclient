// initialisation for MXP

#include "stdafx.h"
#include "..\MUSHclient.h"

#include "..\doc.h"
#include "mxp.h"


#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// colours, tags, entities  (ie. the client-known ones)


// note - these are NOT case-sensitive

tMXP_element MXP_elements [] =
  {

  // open tags

    { "bold",     TAG_OPEN,     MXP_ACTION_BOLD , "" },     // bold
    { "b",        TAG_OPEN,     MXP_ACTION_BOLD , "" },     // bold
    { "high",     TAG_OPEN,     MXP_ACTION_HIGH , "" },     // highlight
    { "h",        TAG_OPEN,     MXP_ACTION_HIGH , "" },     // highlight
    { "underline",TAG_OPEN,     MXP_ACTION_UNDERLINE , "" },// underline
    { "u",        TAG_OPEN,     MXP_ACTION_UNDERLINE , "" },// underline
    { "italic",   TAG_OPEN,     MXP_ACTION_ITALIC , "" },   // italic
    { "i",        TAG_OPEN,     MXP_ACTION_ITALIC , "" },   // italic
    { "em",       TAG_OPEN,     MXP_ACTION_ITALIC , "" },   // em - same as <i>
    { "color",    TAG_OPEN,     MXP_ACTION_COLOR ,          // eg. <color fore=red back=blue>
                                "fore,back" },    
    { "c",        TAG_OPEN,     MXP_ACTION_COLOR ,          // same as COLOR
                                "fore,back" },    
    { "s",        TAG_OPEN | TAG_NOT_IMP,     MXP_ACTION_STRIKE , "" },   // Strikethrough 
    { "strike",   TAG_OPEN | TAG_NOT_IMP,     MXP_ACTION_STRIKE , "" },   // Strikethrough  
    { "strong",   TAG_OPEN,     MXP_ACTION_BOLD , "" },     // Same as Bold                
    { "small",    TAG_OPEN | TAG_NOT_IMP,     MXP_ACTION_SMALL , "" },    // Small text               
    { "tt",       TAG_OPEN | TAG_NOT_IMP,     MXP_ACTION_TT , "" },       // Non-proportional font                
    { "font",     TAG_OPEN,     MXP_ACTION_FONT ,           // Font appearance
                                "color,back,fgcolor,bgcolor" },                          

  // secure tags

    { "frame",    TAG_NOT_IMP,  MXP_ACTION_FRAME , "" },    // frame   (secure)                
    { "dest",     TAG_NOT_IMP,  MXP_ACTION_DEST , "" },     // destination frame  (secure)                 
    { "image",    TAG_COMMAND | TAG_NOT_IMP,  MXP_ACTION_IMAGE ,          // show image  (secure)  
                                "url,fname" },    
    { "filter",   TAG_NOT_IMP,  MXP_ACTION_FILTER , "" },   // sound/image filter  (secure)                 
    { "a",        0,            MXP_ACTION_HYPERLINK ,      // Hyperlink (secure) 
                                "href,xch_cmd,xch_hint" },         
    { "h1",       TAG_NOT_IMP,  MXP_ACTION_H1 , "" },       // Level 1 heading (secure)                 
    { "h2",       TAG_NOT_IMP,  MXP_ACTION_H2 , "" },       // Level 2 heading (secure)                 
    { "h3",       TAG_NOT_IMP,  MXP_ACTION_H3 , "" },       // Level 3 heading (secure)                 
    { "h4",       TAG_NOT_IMP,  MXP_ACTION_H4 , "" },       // Level 4 heading (secure)                 
    { "h5",       TAG_NOT_IMP,  MXP_ACTION_H5 , "" },       // Level 5 heading (secure)                 
    { "h6",       TAG_NOT_IMP,  MXP_ACTION_H6 , "" },       // Level 6 heading (secure)                 
    { "hr",       TAG_COMMAND,  MXP_ACTION_HR , "" },       // Horizontal rule (secure)              
    { "nobr",     TAG_NOT_IMP,  MXP_ACTION_NOBR , "" },     // non-breaking newline               
    { "p",        0,            MXP_ACTION_P , "" },        // Paragraph break (secure)              
    { "script",   TAG_NOT_IMP,  MXP_ACTION_SCRIPT , "" },   // Client script (secure)             
    { "send",     0,            MXP_ACTION_SEND ,           // eg. <send href="go west"> west </send> (secure) 
                                "href,hint,xch_cmd,xch_hint,prompt" },     
    { "ul",       0,            MXP_ACTION_UL , "" },       // Unordered list              
    { "ol",       0,            MXP_ACTION_OL , "" },       // Ordered list
    { "samp",     0,            MXP_ACTION_SAMP , "" },     // Sample text
    { "center",   TAG_NOT_IMP,  MXP_ACTION_CENTER , "" },   // Centre text
    { "var",      0,            MXP_ACTION_VAR , "" },      // set variable
    { "v",        0,            MXP_ACTION_VAR , "" },      // set variable

    // recent additions
    { "gauge",    TAG_NOT_IMP,  MXP_ACTION_GAUGE , "" },    // gauge   (secure)                
    { "stat",     TAG_NOT_IMP,  MXP_ACTION_STAT , "" },     // stat   (secure)                
    { "expire",   TAG_NOT_IMP,  MXP_ACTION_EXPIRE , "" },   // expire (secure)                

    // strictly speaking <LI> isn't a command, but few people bother with </li>
    { "li",       TAG_COMMAND,  MXP_ACTION_LI , "" },       // List item

  // secure, command tags

    { "sound",    TAG_COMMAND | TAG_NOT_IMP,  MXP_ACTION_SOUND , "" },    // play sound                  
    { "music",    TAG_COMMAND | TAG_NOT_IMP,  MXP_ACTION_SOUND , "" },    // play music                  
    { "br",       TAG_COMMAND,  MXP_ACTION_BR , "" },       // Hard Line break (secure)                 
    { "username", TAG_COMMAND,  MXP_ACTION_USER , "" },     // send username (secure)                  
    { "user",     TAG_COMMAND,  MXP_ACTION_USER , "" },     // send username (secure)                   
    { "password", TAG_COMMAND,  MXP_ACTION_PASSWORD , "" }, // send password (secure)                  
    { "pass",     TAG_COMMAND,  MXP_ACTION_PASSWORD , "" }, // send password (secure)                 
    { "relocate", TAG_COMMAND | TAG_NOT_IMP,  MXP_ACTION_RELOCATE , "" }, // causes a new connect to open (secure)                    
    { "version",  TAG_COMMAND,  MXP_ACTION_VERSION , "" },  // version request (secure) 

  // my extension tags

    { "reset",    TAG_COMMAND,  MXP_ACTION_RESET , "" },    // Close all tags             
    { "mxp",      TAG_COMMAND,  MXP_ACTION_MXP , "off" },   // MXP options
    { "support",  TAG_COMMAND,  MXP_ACTION_SUPPORT , "" },  // what we support
    { "option",   TAG_COMMAND,  MXP_ACTION_OPTION , "" },   // what client options are set
    { "afk",      TAG_COMMAND,  MXP_ACTION_AFK , "" },      // how long since they typed something?
    { "recommend_option", TAG_COMMAND,  MXP_ACTION_RECOMMEND_OPTION , "" },   // what options the server recommends


  // Pueblo tags

    { "pre",     TAG_PUEBLO,    MXP_ACTION_PRE    , "" },   // preformatted text (use newlines etc.)
    { "body",    TAG_PUEBLO | TAG_NO_RESET, MXP_ACTION_BODY   , "" },   // body of text 
    { "head",    TAG_PUEBLO | TAG_NO_RESET, MXP_ACTION_HEAD   , "" },   // document header 
    { "html",    TAG_PUEBLO | TAG_NO_RESET, MXP_ACTION_HTML   , "" },   // html mode
    { "title",   TAG_PUEBLO,    MXP_ACTION_TITLE  , "" },   // document title 
    { "img",     TAG_PUEBLO | TAG_COMMAND,  MXP_ACTION_IMG,  // an image ;)
                                "src,xch_mode" },   
    { "xch_page",TAG_PUEBLO | TAG_COMMAND,  MXP_ACTION_XCH_PAGE    , "" },   // starting a new page
    { "xch_pane",TAG_PUEBLO | TAG_COMMAND | TAG_NOT_IMP,  MXP_ACTION_XCH_PANE    , "" },   // starting a new pane

    
  };

// note - these are NOT case-sensitive

// RGB values are in the order 0xRRGGBB

// however we have to swap them as we load them because a COLORREF has
// red as the low-order byte

tMXP_colour MXP_colours [] = 
  {
  { "aliceblue", 0xf0f8ff },     // new in 3.18
  { "antiquewhite", 0xfaebd7 },
  { "aqua", 0x00ffff },
  { "aquamarine", 0x7fffd4 },
  { "azure", 0xf0ffff },
  { "beige", 0xf5f5dc },
  { "bisque", 0xffe4c4 },
  { "black", 0x000000 },
  { "blanchedalmond", 0xffebcd },
  { "blue", 0x0000ff },
  { "blueviolet", 0x8a2be2 },
  { "brown", 0xa52a2a },
  { "burlywood", 0xdeb887 },
  { "cadetblue", 0x5f9ea0 },
  { "chartreuse", 0x7fff00 },
  { "chocolate", 0xd2691e },
  { "coral", 0xff7f50 },
  { "cornflowerblue", 0x6495ed },
  { "cornsilk", 0xfff8dc },
  { "crimson", 0xdc143c },
  { "cyan", 0x00ffff },
  { "darkblue", 0x00008b },
  { "darkcyan", 0x008b8b },
  { "darkgoldenrod", 0xb8860b },
  { "darkgray", 0xa9a9a9 },
  { "darkgreen", 0x006400 },
  { "darkkhaki", 0xbdb76b },
  { "darkmagenta", 0x8b008b },
  { "darkolivegreen", 0x556b2f },
  { "darkorange", 0xff8c00 },
  { "darkorchid", 0x9932cc },
  { "darkred", 0x8b0000 },
  { "darksalmon", 0xe9967a },
  { "darkseagreen", 0x8dbc8f },
  { "darkslateblue", 0x483d8b },
  { "darkslategray", 0x2f4f4f },
  { "darkturquoise", 0x00ded1 },
  { "darkviolet", 0x9400d3 },
  { "deeppink", 0xff1493 },
  { "deepskyblue", 0x00bfff },
  { "dimgray", 0x696969 },
  { "dodgerblue", 0x1e90ff },
  { "firebrick", 0xb22222 },
  { "floralwhite", 0xfffaf0 },
  { "forestgreen", 0x228b22 },
  { "fuchsia", 0xff00ff },
  { "gainsboro", 0xdcdcdc },
  { "ghostwhite", 0xf8f8ff },
  { "gold", 0xffd700 },
  { "goldenrod", 0xdaa520 },
  { "gray", 0x808080 },
  { "green", 0x008000 },
  { "greenyellow", 0xadff2f },
  { "honeydew", 0xf0fff0 },
  { "hotpink", 0xff69b4 },
  { "indianred", 0xcd5c5c },
  { "indigo", 0x4b0082 },
  { "ivory", 0xfffff0 },
  { "khaki", 0xf0e68c },
  { "lavender", 0xe6e6fa },
  { "lavenderblush", 0xfff0f5 },
  { "lawngreen", 0x7cfc00 },
  { "lemonchiffon", 0xfffacd },
  { "lightblue", 0xadd8e6 },
  { "lightcoral", 0xf08080 },
  { "lightcyan", 0xe0ffff },
  { "lightgoldenrodyellow", 0xfafad2 },
  { "lightgreen", 0x90ee90 },
  { "lightgrey", 0xd3d3d3 },
  { "lightgray", 0xd3d3d3 },
  { "lightpink", 0xffb6c1 },
  { "lightsalmon", 0xffa07a },
  { "lightseagreen", 0x20b2aa },
  { "lightskyblue", 0x87cefa },
  { "lightslategray", 0x778899 },
  { "lightsteelblue", 0xb0c4de },
  { "lightyellow", 0xffffe0 },
  { "lime", 0x00ff00 },
  { "limegreen", 0x32cd32 },
  { "linen", 0xfaf0e6 },
  { "magenta", 0xff00ff },
  { "maroon", 0x800000 },
  { "mediumaquamarine", 0x66cdaa },
  { "mediumblue", 0x0000cd },
  { "mediumorchid", 0xba55d3 },
  { "mediumpurple", 0x9370db },
  { "mediumseagreen", 0x3cb371 },
  { "mediumslateblue", 0x7b68ee },
  { "mediumspringgreen", 0x00fa9a },
  { "mediumturquoise", 0x48d1cc },
  { "mediumvioletred", 0xc71585 },
  { "midnightblue", 0x191970 },
  { "mintcream", 0xf5fffa },
  { "mistyrose", 0xffe4e1 },
  { "moccasin", 0xffe4b5 },
  { "navajowhite", 0xffdead },
  { "navy", 0x000080 },
  { "oldlace", 0xfdf5e6 },
  { "olive",  0x808000 },   // new in 3.18
  { "olivedrab", 0x6b8e23 },
  { "orange", 0xffa500 },
  { "orangered", 0xff4500 },
  { "orchid", 0xda70d6 },
  { "palegoldenrod", 0xeee8aa },
  { "palegreen", 0x98fb98 },
  { "paleturquoise", 0xafeeee },
  { "palevioletred", 0xdb7093 },
  { "papayawhip", 0xffefd5 },
  { "peachpuff", 0xffdab9 },
  { "peru", 0xcd853f },
  { "pink", 0xffc8cb },
  { "plum", 0xdda0dd },
  { "powderblue", 0xb0e0e6 },
  { "purple", 0x800080 },
  { "red", 0xff0000 },
  { "rosybrown", 0xbc8f8f },
  { "royalblue", 0x4169e1 },
  { "saddlebrown", 0x8b4513 },
  { "salmon", 0xfa8072 },
  { "sandybrown", 0xf4a460 },
  { "seagreen", 0x2e8b57 },
  { "seashell", 0xfff5ee },
  { "sienna", 0xa0522d },
  { "silver", 0xc0c0c0 },
  { "skyblue", 0x87ceeb },
  { "slateblue", 0x6a5acd },
  { "slategray", 0x708090 },   // new in 3.18
  { "snow", 0xfffafa },
  { "springgreen", 0x00ff7f },
  { "steelblue", 0x4682b4 },
  { "tan", 0xd2b48c },
  { "teal", 0x008080 },
  { "thistle", 0xd8bfd8 },
  { "tomato", 0xff6347 },
  { "turquoise", 0x40e0d0 },
  { "violet", 0xee82ee },
  { "wheat", 0xf5deb3 },
  { "white", 0xffffff },
  { "whitesmoke", 0xf5f5f5 },
  { "yellow", 0xffff00 },
  { "yellowgreen", 0x9acd32 },
 }; // end of colours list

 // note - these ARE case-sensitive

char * character_entities [] = 
  {
"lt", "<",          "micro", "&#181;",  "Icirc", "&#206;",  "ccedil", "&#231;",
"gt", ">",          "para", "&#182;",   "Iuml", "&#207;",   "egrave", "&#232;",  
"amp", "&",         "middot", "&#183;", "ETH", "&#208;",    "eacute", "&#233;",
"quot", "\"",       "cedil", "&#184;",  "Ntilde", "&#209;", "ecirc", "&#234;", 
"nbsp", "&#160;",   "sup1", "&#185;",   "Ograve", "&#210;", "euml", "&#235;",  
"iexcl", "&#161;",  "ordm", "&#186;",   "Oacute", "&#211;", "igrave", "&#236;",
"cent", "&#162;",   "raquo", "&#187;",  "Ocirc", "&#212;",  "iacute", "&#237;",
"pound", "&#163;",  "frac14", "&#188;", "Otilde", "&#213;", "icirc", "&#238;", 
"curren", "&#164;", "frac12", "&#189;", "Ouml", "&#214;",   "iuml", "&#239;",  
"yen", "&#165;",    "frac34", "&#190;", "times", "&#215;",  "eth", "&#240;",   
"brvbar", "&#166;", "iquest", "&#191;", "Oslash", "&#216;", "ntilde", "&#241;",
"sect", "&#167;",   "Agrave", "&#192;", "Ugrave", "&#217;", "ograve", "&#242;",
"uml", "&#168;",    "Aacute", "&#193;", "Uacute", "&#218;", "oacute", "&#243;",
"copy", "&#169;",   "Acirc", "&#194;",  "Ucirc", "&#219;",  "ocirc", "&#244;", 
"ordf", "&#170;",   "Atilde", "&#195;", "Uuml", "&#220;",   "otilde", "&#245;",
"laquo", "&#171;",  "Auml", "&#196;",   "Yacute", "&#221;", "ouml", "&#246;",  
"not", "&#172;",    "Aring", "&#197;",  "THORN", "&#222;",  "divide", "&#247;",
"shy", "&#173;",    "AElig", "&#198;",  "szlig", "&#223;",  "oslash", "&#248;",
"reg", "&#174;",    "Ccedil", "&#199;", "agrave", "&#224;", "ugrave", "&#249;",
"macr", "&#175;",   "Egrave", "&#200;", "aacute", "&#225;", "uacute", "&#250;",
"deg", "&#176;",    "Eacute", "&#201;", "acirc", "&#226;",  "ucirc", "&#251;", 
"plusmn", "&#177;", "Ecirc", "&#202;",  "atilde", "&#227;", "uuml",  "&#252;",  
"sup2", "&#178;",   "Euml", "&#203;",   "auml", "&#228;",   "yacute", "&#253;",
"sup3", "&#179;",   "Igrave", "&#204;", "aring", "&#229;",  "thorn", "&#254;", 
"acute", "&#180;",  "Iacute", "&#205;", "aelig", "&#230;",  "yuml",  "&#255;",

"apos", "\'",   // new in 3.20
    
"", "",             // end of table marker
      
  };


void CMUSHclientApp::MXP_LoadColours (void)
  {

CColours * colour_item;
CString strName;

  for (int i = 0; i < NUMITEMS (MXP_colours); i++)
    {
    strName = MXP_colours [i].pName;
    strName.MakeLower ();     // ensure lower case
    m_ColoursMap.SetAt (strName, colour_item = new CColours);
    colour_item->strName = strName;
    // colours seem to be inverted in my table above
    colour_item->iColour = RGB (
                      GetBValue (MXP_colours [i].iColour),
                      GetGValue (MXP_colours [i].iColour),
                      GetRValue (MXP_colours [i].iColour)
                      );    

// temporary, for displaying colours
/*

  CString str = CFormat (
    "<tr><th width=\"20*\" align=right>%s</th>"
    "<td width=\"30*\">R=%i, G=%i, B=%i</td>"
    "<td bgcolor=\"#%02X%02X%02X\">&nbsp;</td></tr>", 
                      (LPCTSTR) strName,    // column 1

                      GetBValue (MXP_colours [i].iColour),  // column 2
                      GetGValue (MXP_colours [i].iColour),
                      GetRValue (MXP_colours [i].iColour),

                      GetBValue (MXP_colours [i].iColour),  // column 3
                      GetGValue (MXP_colours [i].iColour),
                      GetRValue (MXP_colours [i].iColour)
                      );

  str += ENDLINE;

  TRACE (str);

*/

// end temp

    
    }   // end of doing each one

  } // end of CMUSHclientApp::LoadColours 


// add all entities to map

void CMUSHclientApp::MXP_LoadEntities (void)
  {
CString strName;
CString strReplacement;

  for (int j = 0; character_entities [j] [0] != 0; j += 2)
    {
    strName = character_entities [j];
    strReplacement = character_entities [j + 1];
    if (strReplacement.Left (2) == "&#" &&
        strReplacement.Right (1) == ";")
      {
      const char * p = strReplacement;
      int iResult = 0;

      for (p +=2; *p != ';'; p++)
        {
        if (!isdigit (*p))
          break;
        iResult *= 10;
        iResult += *p - '0';
        } // end of building up character

      unsigned char cOneCharacterLine [2] = { (unsigned char) iResult, 0};
      strReplacement = (char *) cOneCharacterLine;
      } // end of converting #&xxx;

    m_EntityMap.SetAt (strName, strReplacement);

// temporary, for displaying entities

/*
    int i;
    CString strValue = character_entities [j + 1];
    if (strValue.Left (2) == "&#")
      i = atoi (strValue.Mid (2));
    else
      i = strValue [0];


    CString str = CFormat (
      "<tr><th align=left>&amp;%s;</th>"
      "<td align=right>%i</td>"
      "<td>%s</td></tr>", 
                        (LPCTSTR) character_entities [j],    // column 1 - name
                        i,  // column 2  - ANSI code
                        (LPCTSTR) character_entities [j + 1]  // column 3 - character itself
                        );

    str += ENDLINE;

    TRACE (str);

*/

    }

  } // end of CMUSHclientApp::LoadEntities 


void CMUSHclientApp::MXP_LoadElements (void)
  {

CAtomicElement * element_item;

  for (int i = 0; i < NUMITEMS (MXP_elements); i++)
    {
    m_ElementMap.SetAt (MXP_elements [i].pName, element_item = new CAtomicElement);
    element_item->strName = MXP_elements [i].pName;
    element_item->strArgs = MXP_elements [i].pArgs;
    element_item->iFlags = MXP_elements [i].iFlags;
    element_item->iAction = MXP_elements [i].iAction;
    }   // end of doing each one

  } // end of CMUSHclientApp::LoadAtomicElements 
