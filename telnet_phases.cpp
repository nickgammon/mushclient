// telnet_phases.cpp - handle telnet phases in incoming text stream

#include "stdafx.h"
#include "MUSHclient.h"

#include "doc.h"

/* Phases for processing input stream ...


  To allow for these to be broken between packets we only do one
  character at a time, and maintain a "state" (Phase) which indicates
  how far through the sequence we are.

  The two "triggering" codes we might get in normal text (ie. phase NONE) are
  ESC or IAC.

  Tested (5 Feb 2010):

  * MCCP v1
  * MCCP v2
  * IAC GA              \ff\f9
  * IAC EOR             \ff\ef
  * IAC IAC inside subnegotiation   \FF\FD\66 \ff\fa\66hello\ff\ffboys\ff\f0
  * IAC IAC in normal text      what\ff\ffever
  * MXP
  * Charset: \FF\FD\2A \ff\fa\2A\01,UTF-8,US-ASCII\ff\f0
  * terminal type: \FF\FD\18 \ff\fa\18\01\ff\f0
  * NAWS: \FF\FD\1f \ff\fa\1f\ff\f0
  * Chat system


*/

// ESC x

void CMUSHclientDoc::Phase_ESC (const unsigned char c)
  {
  if (c == '[')
    {
    m_phase = DOING_CODE;
    m_code = 0;
    }
  else
    m_phase = NONE;

  } // end of Phase_ESC

// ANSI - We have ESC [ x

void CMUSHclientDoc::Phase_ANSI (const unsigned char c)
  {
  if (isdigit (c))
    {
    m_code *= 10;
    m_code += c - '0';
    }
  else if (c == 'm')
    {
    if (m_phase != DOING_CODE)
      Interpret256ANSIcode (m_code);
    else
      InterpretANSIcode (m_code);
    m_phase = NONE;
    }
  else if (c == ';')
    {
    if (m_phase != DOING_CODE)
      Interpret256ANSIcode (m_code);
    else
      InterpretANSIcode (m_code);
    m_code = 0;
    }
  else if (c == 'z')    // MXP line security mode
    {
    if (m_code == eMXP_reset)
      MXP_Off ();
    else
      {
//      TRACE1 ("\nMXP security mode now %i\n", m_code);
      MXP_mode_change (m_code);
      }
    m_phase = NONE;
    }
  else
    m_phase = NONE;
  } // end of Phase_ANSI

// IAC - we have IAC x

void CMUSHclientDoc::Handle_IAC_GA ()
  {
  SendToAllPluginCallbacks (ON_PLUGIN_IAC_GA);
  }   // end of  CMUSHclientDoc::Handle_IAC_GA

void CMUSHclientDoc::Phase_IAC (unsigned char & c)
  {
  char * p;
  unsigned char new_c = 0;    // returning zero stops further processing of c

  switch (c)
    {
    case EOR                 : 
          m_phase = NONE; 
          p = "EOR"; 
          if (m_bConvertGAtoNewline)
            new_c = '\n';
          m_last_line_with_IAC_GA = m_total_lines;
          Handle_IAC_GA ();
          break;
    case GO_AHEAD            : 
          m_phase = NONE; 
          p = "GA"; 
          if (m_bConvertGAtoNewline)
            new_c = '\n';
          m_last_line_with_IAC_GA = m_total_lines;
          Handle_IAC_GA ();
          break;

    case SE                  : m_phase = NONE;      p = "SE"; break;
    case NOP                 : m_phase = NONE;      p = "NOP"; break;
    case DATA_MARK           : m_phase = NONE;      p = "DM"; break;
    case BREAK               : m_phase = NONE;      p = "BRK"; break;
    case INTERRUPT_PROCESS   : m_phase = NONE;      p = "IP"; break;
    case ABORT_OUTPUT        : m_phase = NONE;      p = "AO"; break;
    case ARE_YOU_THERE       : m_phase = NONE;      p = "AYT"; break;
    case ERASE_CHARACTER     : m_phase = NONE;      p = "EC"; break;
    case ERASE_LINE          : m_phase = NONE;      p = "EL"; break;
    case SB                  : m_phase = HAVE_SB;   p = "SB"; break;
    case WILL                : m_phase = HAVE_WILL; p = "WILL"; break;
    case WONT                : m_phase = HAVE_WONT; p = "WONT"; break;
    case DO                  : m_phase = HAVE_DO;   p = "DO"; break;
    case DONT                : m_phase = HAVE_DONT; p = "DONT"; break;
    default                  : m_phase = NONE;      p = "none"; break;
    } // end of switch
  TRACE1 ("<%s>", p);
  m_subnegotiation_type = 0;    // no subnegotiation type yet
  c = new_c;

  } // end of Phase_IAC

// send to all plugins: function OnPluginTelnetRequest (num, type)
//    eg. num = 200, type = DO
// They reply true or false to handle or not handle that telnet type
 
bool CMUSHclientDoc::Handle_Telnet_Request (const int iNumber, const string sType)

  {

  return SendToAllPluginCallbacks (ON_PLUGIN_TELNET_REQUEST,
                                    iNumber,
                                    sType,
                                    true,     // stop on true response
                                    false);

  } // end of CMUSHclientDoc::Handle_Telnet_Request 

void CMUSHclientDoc::Send_IAC_DO (const unsigned char c)
  {
  // if we are already in a mode do not agree again - see RFC 854 
  // and forum subject 3061                           

  if (m_bClient_sent_IAC_DO [c])
      return;

  unsigned char do_do_it [3]   = { IAC, DO, c };

  TRACE1 ("\nSending IAC DO <%d>\n", c);
  SendPacket (do_do_it, sizeof do_do_it);
  m_bClient_sent_IAC_DO [c]   = true;
  m_bClient_sent_IAC_DONT [c] = false;

  } // end of CMUSHclientDoc::Send_IAC_DO

void CMUSHclientDoc::Send_IAC_DONT (const unsigned char c)
  {
  // if we are already in a mode do not agree again - see RFC 854 
  // and forum subject 3061                           
  if (m_bClient_sent_IAC_DONT [c])
      return;

  unsigned char dont_do_it [3] = { IAC, DONT, c };

  TRACE1 ("\nSending IAC DONT <%d>\n", c);
  SendPacket (dont_do_it, sizeof dont_do_it);
  m_bClient_sent_IAC_DONT [c] = true;
  m_bClient_sent_IAC_DO [c]   = false;

  } // end of CMUSHclientDoc::Send_IAC_DONT

void CMUSHclientDoc::Send_IAC_WILL (const unsigned char c)
  {
  // if we are already in a mode do not agree again - see RFC 854 
  // and forum subject 3061                           
  if (m_bClient_sent_IAC_WILL [c])
      return;

  unsigned char will_do_it [3]   = { IAC, WILL, c };

  TRACE1 ("\nSending IAC WILL <%d>\n", c);
  SendPacket (will_do_it, sizeof will_do_it);
  m_bClient_sent_IAC_WILL [c] = true;
  m_bClient_sent_IAC_WONT [c] = false;

  } // end of CMUSHclientDoc::Send_IAC_WILL

void CMUSHclientDoc::Send_IAC_WONT (const unsigned char c)
  {
  // if we are already in a mode do not agree again - see RFC 854 
  // and forum subject 3061                           
  if (m_bClient_sent_IAC_WONT [c])
      return;

  unsigned char wont_do_it [3] = { IAC, WONT, c };

  TRACE1 ("\nSending IAC WONT <%d>\n", c);
  SendPacket (wont_do_it, sizeof wont_do_it);
  m_bClient_sent_IAC_WONT [c] = true;
  m_bClient_sent_IAC_WILL [c] = false;

  } // end of CMUSHclientDoc::Send_IAC_WONT


// WILL - we have IAC WILL x   - reply DO or DONT (generally based on client option settings)
// for unknown types we query plugins: function OnPluginTelnetRequest (num, type) 
//    eg. num = 200, type = WILL
// They reply true or false to handle or not handle that telnet type

void CMUSHclientDoc::Phase_WILL (const unsigned char c)
  {

// telnet negotiation : in response to WILL, we say DONT
// (except for compression, MXP, TERMINAL_TYPE and SGA), we *will* handle that)

  TRACE1 ("<%d>", c);
  m_phase = NONE;  // back to normal text after this character

  m_nCount_IAC_WILL++;
  m_bClient_got_IAC_WILL [c] = true;

  switch (c)
    {
    case TELOPT_COMPRESS2:
    case TELOPT_COMPRESS:
      // initialise compression library if not already decompressing
      if (!m_bCompressInitOK && !m_bCompress)
        m_bCompressInitOK = InitZlib (m_zCompress);
      if (m_bCompressInitOK && !m_bDisableCompression)
        {
        if (!m_CompressOutput)
          m_CompressOutput = (Bytef *) malloc (m_nCompressionOutputBufferSize);
        if (!m_CompressInput)
          m_CompressInput = (Bytef *) malloc (COMPRESS_BUFFER_LENGTH);

        if (m_CompressOutput && !m_CompressInput)
          {
          free (m_CompressOutput);    // only got one? may as well free it
          m_CompressOutput = NULL;
          }

        if (m_CompressOutput && m_CompressInput &&     // we got memory - we can do it
            !(c == TELOPT_COMPRESS && m_bSupports_MCCP_2)) // don't agree to MCCP1 and MCCP2
          {
          Send_IAC_DO (c);
          if (c == TELOPT_COMPRESS2)
            m_bSupports_MCCP_2 = true;
          }
        else
          {   // not enough memory or already agreed to MCCP 2 - no compression
          Send_IAC_DONT (c);
          }
        }   // end of compression wanted and zlib engine initialised
      else
        {
        Send_IAC_DONT (c);
        }
      break;    // end of TELOPT_COMPRESS

    // here for SGA (Suppress GoAhead) 
    case SGA:
          Send_IAC_DO (c);
          break;  // end of SGA 

    // here for TELOPT_MUD_SPECIFIC 
    case TELOPT_MUD_SPECIFIC:
          Send_IAC_DO (c);
          break;  // end of TELOPT_MUD_SPECIFIC 

    case TELOPT_ECHO:
          if (!m_bNoEchoOff)
            {
            m_bNoEcho = true;
            TRACE ("Echo turned off\n");
            Send_IAC_DO (c);
            }
          else
            Send_IAC_DONT (c);
          break; // end of TELOPT_ECHO

    case TELOPT_MXP:
          if (m_iUseMXP == eNoMXP)
            {
            Send_IAC_DONT (c);
            }     // end of no MXP wanted
          else
            {
            Send_IAC_DO (c);
            if (m_iUseMXP == eQueryMXP)     // turn MXP on now
              MXP_On ();
            } // end of MXP wanted
          break;  // end of MXP
          
    // here for EOR (End of record)
    case WILL_END_OF_RECORD:
          if (m_bConvertGAtoNewline)
            Send_IAC_DO (c);
          else
            Send_IAC_DONT (c);
          break;  // end of WILL_END_OF_RECORD


    default:
        if (Handle_Telnet_Request (c, "WILL"))
          {
          Send_IAC_DO (c);
          Handle_Telnet_Request (c, "SENT_DO");
          }
        else
          Send_IAC_DONT (c);
        break;  // end of others

    } // end of switch


  } // end of Phase_WILL

// Received: IAC WONT x

void CMUSHclientDoc::Phase_WONT (const unsigned char c)
  {

// telnet negotiation : in response to WONT, we say DONT

  TRACE1 ("<%d>", c);
  m_phase = NONE;

  m_nCount_IAC_WONT++;
  m_bClient_got_IAC_WONT [c] = true;

  switch (c)
    {
    case TELOPT_ECHO:
        if (!m_bNoEchoOff)
          {
          m_bNoEcho = false;
          TRACE ("Echo turned on\n");
          }
        Send_IAC_DONT (c);
        break; // end of TELOPT_ECHO

    default:
      Send_IAC_DONT (c);
      break;
    } // end of switch
  } // end of Phase_WONT

// Received: IAC DO x

// for unknown types we query plugins: function OnPluginTelnetRequest (num, type) 
//    eg. num = 200, type = DO
// They reply true or false to handle or not handle that telnet type

void CMUSHclientDoc::Phase_DO (const unsigned char c)
  {
// telnet negotiation : in response to DO, we say WILL for:  
//  <102> (Aardwolf), SGA, echo, NAWS, CHARSET, MXP and Terminal type
// for others we query plugins to see if they want to handle it or not

  TRACE1 ("<%d>", c);
  m_phase = NONE;

  m_nCount_IAC_DO++;
  m_bClient_got_IAC_DO [c] = true;

  switch (c)
    {
    case SGA:
    case TELOPT_MUD_SPECIFIC:
    case TELOPT_ECHO:
    case TELOPT_CHARSET:
        Send_IAC_WILL (c);
        break; // end of things we will do 
            
    // for MTTS start back at sequence 0
    case TELOPT_TERMINAL_TYPE:   
        m_ttype_sequence = 0;
        Send_IAC_WILL (c);
        break;

    case TELOPT_NAWS:
      // option off - must be server initiated
      if (m_bNAWS)
        {
        Send_IAC_WILL (c);
        m_bNAWS_wanted = true;
        SendWindowSizes (m_nWrapColumn);
        }
      else
        Send_IAC_WONT (c);
      break;

    case TELOPT_MXP:
          if (m_iUseMXP == eNoMXP)
            Send_IAC_WONT (c);
          else
            {
            Send_IAC_WILL (c);
            if (m_iUseMXP == eQueryMXP)     // turn MXP on now
              MXP_On ();
            } // end of MXP wanted
          break;  // end of MXP

    default:
          if (Handle_Telnet_Request (c, "DO"))
            {
            Send_IAC_WILL (c);
            Handle_Telnet_Request (c, "SENT_WILL");
            }
          else
            Send_IAC_WONT (c);
          break;    // end of others
    }   // end of switch

  } // end of Phase_DO

// Received: IAC DONT x

void CMUSHclientDoc::Phase_DONT (const unsigned char c)
  {
// telnet negotiation : in response to DONT, we say WONT

  TRACE1 ("<%d>", c);
  m_phase = NONE;
  Send_IAC_WONT (c);

  m_nCount_IAC_DONT++;
  m_bClient_got_IAC_DONT [c] = true;

  switch (c)
    {

    case TELOPT_MXP:
          if (m_bMXP)
            MXP_Off (true);
          break;  // end of MXP

    // for MTTS start back at sequence 0
    case TELOPT_TERMINAL_TYPE:   
        m_ttype_sequence = 0;
        break;

    }   // end of switch

  } // end of Phase_DONT

// SUBNEGOTIATION - we have IAC SB c 
// remember c (the type) and start collecting the data, as in:
// IAC SB c <data> IAC SE

void CMUSHclientDoc::Phase_SB (const unsigned char c)
  {
  TRACE1 ("<%d>", c);

  // note IAC SB COMPRESS is a special case because they forgot to specify
  // the IAC SE, and thus we can't use normal negotiation
  if (c == TELOPT_COMPRESS)
    m_phase = HAVE_COMPRESS;
  else
    {
    m_subnegotiation_type = c;
    m_IAC_subnegotiation_data.erase ();
    m_phase = HAVE_SUBNEGOTIATION;
    }
  } // end of CMUSHclientDoc::Phase_SB 

// SUBNEGOTIATION - we have IAC SB c (data)
// if we get an IAC remember it, because it may or may not be followed by IAC or SE
void CMUSHclientDoc::Phase_SUBNEGOTIATION (const unsigned char c)
  {

  if (c == IAC)
    {
    // have IAC SB x <data> IAC
    // we may or may not have another IAC or possibly an SE
    m_phase = HAVE_SUBNEGOTIATION_IAC;
    }
  else
    // just collect the data until IAC SE
    m_IAC_subnegotiation_data += c;


  } // end of Phase_SUBNEGOTIATION

// SUBNEGOTIATION - we have IAC SB x (data) IAC c
// if the c after IAC is IAC then that becomes a single IAC (which we store now)
// otherwise it should be SE, and we assume it is, otherwise we have an invalid sequence
void CMUSHclientDoc::Phase_SUBNEGOTIATION_IAC (const unsigned char c)
  {

  if (c == IAC)
    {
    // have IAC SB x <data> IAC IAC
    // store the single IAC
    m_IAC_subnegotiation_data += c;
    // press on with subnegotiation
    m_phase = HAVE_SUBNEGOTIATION;
    return;
    }

  // see: http://www.gammon.com.au/forum/?id=10043
  // we have to assume that anything other than IAC is a SE, because 
  // the spec is silent on what to do otherwise
  if (c == SE)        // end of subnegotiation
    TRACE ("<SE>");
  else
    TRACE1 ("<%d> (invalid)", c);

  m_phase = NONE;      // negotiation is over, next byte is plain text

  m_nCount_IAC_SB++;

  // subnegotiation is complete ...
  // we have IAC SB <m_subnegotiation_type> <m_IAC_subnegotiation_data> IAC SE

  switch (m_subnegotiation_type)
    {
    case TELOPT_COMPRESS2:      
      Handle_TELOPT_COMPRESS2 ();     
      break;

    case TELOPT_MXP:            
      Handle_TELOPT_MXP ();           
      break;

    case TELOPT_TERMINAL_TYPE:  
      Handle_TELOPT_TERMINAL_TYPE (); 
      break;

    case TELOPT_CHARSET:        
      Handle_TELOPT_CHARSET ();       
      break;


    case TELOPT_MUD_SPECIFIC:   
        Handle_TELOPT_MUD_SPECIFIC ();  
    // NOTE: no break, fall through and also do the default

    default:
      {
      SendToAllPluginCallbacks (ON_PLUGIN_TELNET_SUBNEGOTIATION,
                                m_subnegotiation_type,
                                m_IAC_subnegotiation_data,
                                false,
                                false);

      }
      break;  // end of default

    } // end of switch

  } // end of  CMUSHclientDoc::Phase_SUBNEGOTIATION_IAC 


// turn MCCP v2 on
void CMUSHclientDoc::Handle_TELOPT_COMPRESS2 ()
  {
  CString strMessage;

  m_iMCCP_type = 2;

  // initialise compression library if not already done
  if (!m_bCompressInitOK && !m_bCompress)
    m_bCompressInitOK = InitZlib (m_zCompress);

  if (!(m_bCompressInitOK && m_CompressOutput && m_CompressInput))
    strMessage = Translate ("Cannot process compressed output. World closed.");
  else
    {

    int izError = inflateReset (&m_zCompress);

    if (izError == Z_OK)
      {
      m_bCompress  = true;
      TRACE ("Compression on\n");
      return;
      }

    if (m_zCompress.msg)
      strMessage = TFormat ("Could not reset zlib decompression engine: %s",
                               m_zCompress.msg);
    else
      strMessage = TFormat ("Could not reset zlib decompression engine: %i",
                               izError);
    } 

  OnConnectionDisconnect ();    // close the world
  UMessageBox (strMessage, MB_ICONEXCLAMATION);
  } // end of CMUSHclientDoc::Handle_TELOPT_COMPRESS2


// turn MXP on, if required on subnegotiation
void CMUSHclientDoc::Handle_TELOPT_MXP ()
  {
  if (m_iUseMXP == eOnCommandMXP)   // if wanted now
    MXP_On ();
  } // end of CMUSHclientDoc::Handle_TELOPT_MXP ()

// IAC SB CHARSET REQUEST DELIMITER <name> DELIMITER
/*
Server sends:  IAC DO CHARSET
Client sends:  IAC WILL CHARSET
Server sends:  IAC SB CHARSET REQUEST DELIM NAME IAC SE
Client sends:  IAC SB CHARSET ACCEPTED NAME IAC SE
or
Client sends:  IAC SB CHARSET REJECTED IAC SE

where:

  CHARSET: 0x2A
  REQUEST: 0x01
  ACCEPTED:0x02
  REJECTED:0x03
  DELIM:   some character that does not appear in the charset name, other than IAC, eg. comma, space
  NAME:    the character string "UTF-8" (or some other name like "S-JIS")

*/

void CMUSHclientDoc::Handle_TELOPT_CHARSET ()
  {
  // must have at least REQUEST DELIM NAME [ DELIM NAME2 ...]
  if (m_IAC_subnegotiation_data.size () < 3)
    return;  

  int tt = m_IAC_subnegotiation_data [0];

  if (tt != 1) 
    return;  // not a REQUEST

  string delim = m_IAC_subnegotiation_data.substr (1, 1);

  vector <string> v;
  StringToVector (m_IAC_subnegotiation_data.substr (2), v, delim, false);

  bool found = false;
  CString strCharset = "US-ASCII"; // default

  if (m_font)
    {

    // hack! ugh.
    if (m_bUTF_8)
       strCharset = "UTF-8";

    for (vector<string>::const_iterator i = v.begin (); i != v.end (); i++)
      if (i->c_str () == strCharset)
        {
        found = true;

        unsigned char p1 [] = { IAC, SB, TELOPT_CHARSET, 2 };  // 2 = accepted
        unsigned char p2 [] = { IAC, SE }; 
        unsigned char sResponse [40];
        int iLength = 0;

        // build up response, eg. IAC, SB, TELOPT_CHARSET, 2, "UTF-8", IAC, SE 

        // preamble
        memcpy (sResponse, p1, sizeof p1);
        iLength += sizeof p1;

        // ensure max of 20 so we don't overflow the field
        CString strTemp = strCharset.Left (20);

        memcpy (&sResponse [iLength], strTemp, strTemp.GetLength ());
        iLength += strTemp.GetLength ();

        // postamble
        memcpy (&sResponse [iLength], p2, sizeof p2);
        iLength += sizeof p2;

        SendPacket (sResponse, iLength);

        }
    } // end of having an output font

  if (!found)
    {
    unsigned char p [] = { IAC, SB, TELOPT_CHARSET, 3, IAC, SE };    // 3 = rejected
    SendPacket (p, sizeof p);
    }  // end of charset not in use


  } // end of CMUSHclientDoc::Handle_TELOPT_CHARSET ()


// stuff for Aardwolf (telopt 102) - call specific plugin handler: OnPluginTelnetOption
void CMUSHclientDoc::Handle_TELOPT_MUD_SPECIFIC ()
  {
  CString strReceived (m_IAC_subnegotiation_data.c_str ());
  SendToAllPluginCallbacks (ON_PLUGIN_TELNET_OPTION, strReceived);
  } // end of CMUSHclientDoc::Handle_TELOPT_MUD_SPECIFIC ()

#define TTYPE_IS 0
#define TTYPE_SEND 1

// terminal type request
void CMUSHclientDoc::Handle_TELOPT_TERMINAL_TYPE ()
  {

  int tt = m_IAC_subnegotiation_data [0];

  if (tt != TTYPE_SEND) 
    return;  // not a SEND

  TRACE ("<SEND>");
  // we reply: IAC SB TERMINAL-TYPE IS ... IAC SE
  // see: RFC 930 and RFC 1060
  // also see: http://tintin.sourceforge.net/mtts/

  unsigned char p1 [] = { IAC, SB, TELOPT_TERMINAL_TYPE, TTYPE_IS }; 
  unsigned char p2 [] = { IAC, SE }; 
  unsigned char sResponse [40];
  int iLength = 0;

  // build up response, eg. IAC, SB, TELOPT_TERMINAL_TYPE, 0, "MUSHCLIENT", IAC, SE 

  // preamble
  memcpy (sResponse, p1, sizeof p1);
  iLength += sizeof p1;

  // ensure max of 20 so we don't overflow the field
  CString strTemp;
  

  /*
  On the first TTYPE SEND request the client should return its name, preferably without a version number and in all caps.

  On the second TTYPE SEND request the client should return a terminal type, preferably in all caps. 
    Console clients should report the name of the terminal emulator, 
    other clients should report one of the four most generic terminal types.

      "DUMB"              Terminal has no ANSI color or VT100 support.
      "ANSI"              Terminal supports all ANSI color codes. Supporting blink and underline is optional.
      "VT100"             Terminal supports most VT100 codes, including ANSI color codes.
      "XTERM"             Terminal supports all VT100 and ANSI color codes, xterm 256 colors, mouse tracking, and the OSC color palette.

  If 256 color detection for non MTTS compliant servers is a must it's an option 
    to report "ANSI-256COLOR", "VT100-256COLOR", or "XTERM-256COLOR". 
    The terminal is expected to support VT100, mouse tracking, and the OSC color palette if "XTERM-256COLOR" is reported.

  On the third TTYPE SEND request the client should return MTTS followed by a bitvector. The bit values and their names are defined below.

          1 "ANSI"              Client supports all ANSI color codes. Supporting blink and underline is optional.
          2 "VT100"             Client supports most VT100 codes.
          4 "UTF-8"             Client is using UTF-8 character encoding.
          8 "256 COLORS"        Client supports all xterm 256 color codes.
         16 "MOUSE TRACKING"    Client supports xterm mouse tracking.
         32 "OSC COLOR PALETTE" Client supports the OSC color palette.
         64 "SCREEN READER"     Client is using a screen reader.
        128 "PROXY"             Client is a proxy allowing different users to connect from the same IP address.
         
  */

  switch (m_ttype_sequence)
    {
    case 0:  
        strTemp = m_strTerminalIdentification.Left (20);
        m_ttype_sequence++;
        break;

    case 1:
        strTemp = "ANSI";
        m_ttype_sequence++;
        break;

    case 2:
        if (m_bUTF_8)
          strTemp = "MTTS 13";
        else
          strTemp = "MTTS 9";
        break;

    } // end of switch

  memcpy (&sResponse [iLength], strTemp, strTemp.GetLength ());
  iLength += strTemp.GetLength ();

  // postamble
  memcpy (&sResponse [iLength], p2, sizeof p2);
  iLength += sizeof p2;

  SendPacket (sResponse, iLength);

  } // end of CMUSHclientDoc::Handle_TELOPT_TERMINAL_TYPE ()


// COMPRESSION - we have IAC SB COMPRESS x
void CMUSHclientDoc::Phase_COMPRESS (const unsigned char c)
  {
  if (c == WILL)      // should get COMPRESS WILL
    {
    TRACE ("<WILL>");
    m_phase = HAVE_COMPRESS_WILL;
    }
  else
    {
    m_phase = NONE;   // error
    TRACE1 ("<%d>", c);
    }
  }

// COMPRESSION - we have IAC SB COMPRESS IAC/WILL x   (MCCP v1)

// we will return one of:
//  0 - error in starting compression - close world and display strMessage
//  1 - got IAC or unexpected input, do nothing
//  2 - compression OK - prepare for it

void CMUSHclientDoc::Phase_COMPRESS_WILL (const unsigned char c)
  {
  if (c == SE)        // end of subnegotiation
    {        
    TRACE ("<SE>");

    CString strMessage;

    m_iMCCP_type = 1;

    // initialise compression library if not already done
    if (!m_bCompressInitOK && !m_bCompress)
      m_bCompressInitOK = InitZlib (m_zCompress);

    if (!(m_bCompressInitOK && m_CompressOutput && m_CompressInput))
      strMessage = Translate ("Cannot process compressed output. World closed.");
    else
      {

      int izError = inflateReset (&m_zCompress);

      if (izError == Z_OK)
        {
        m_bCompress  = true;
        TRACE ("Compression on\n");
        return;
        }

      if (m_zCompress.msg)
        strMessage = TFormat ("Could not reset zlib decompression engine: %s",
                                 m_zCompress.msg);
      else
        strMessage = TFormat ("Could not reset zlib decompression engine: %i",
                                 izError);
      } 

    OnConnectionDisconnect ();    // close the world
    UMessageBox (strMessage, MB_ICONEXCLAMATION);
    }   // end of IAC SB COMPRESS WILL/IAC SE

  
  // not SE? error
  TRACE1 ("<%d>", c);
  m_phase = NONE;
  } // end of Phase_COMPRESS_WILL

// in UTF-8 mode, if we get a "bad" UTF-8 character we assume it is standard ANSI
// and convert it from ANSI into Unicode, and then into UTF-8 and output that instead
void CMUSHclientDoc::OutputBadUTF8characters (void)
  {
  for (int i = 0; m_UTF8Sequence [i]; i++)
    {
    // convert ANSI to Unicode:
    WCHAR sUnicode [1];
    MultiByteToWideChar (CP_THREAD_ACP, MB_PRECOMPOSED, (const char *) &m_UTF8Sequence [i], 1, sUnicode, 1); 
    // now convert Unicode to UTF8
    char sOutput [5];
    memset (sOutput, 0, sizeof sOutput);  // ensure trailing null
    WideCharToMultiByte (CP_UTF8, 0, sUnicode, 1, sOutput, sizeof sOutput, NULL, NULL);
    AddToLine (sOutput, 0); 
    m_cLastChar = m_UTF8Sequence [i];
    }

  m_phase = NONE;
  }  // end of CMUSHclientDoc::OutputBadUTF8characters

// test data: testing \C5\87\C4\A8\C4\86\C4\B6 Gammon and now: \C6 <---

// here when getting second or subsequent bytes of a UTF8 character
void CMUSHclientDoc::Phase_UTF8 (const unsigned char c)
  {

  // append to our UTF8 sequence
  int i = 0;
  while (m_UTF8Sequence [i])
    i++;
  m_UTF8Sequence [i] = c;
  m_UTF8Sequence [i + 1] = 0;  // null terminator

  if ((c & 0xC0) != 0x80)
    {
    OutputBadUTF8characters ();
    return;
    }

  // we are waiting for less of them
  m_iUTF8BytesLeft--;

  // more to go
  if (m_iUTF8BytesLeft > 0)
    return;

  // check the sequence

  int erroroffset;
  int iBad = _pcre_valid_utf ((const unsigned char *) m_UTF8Sequence, strlen ((const char *) m_UTF8Sequence), &erroroffset);
  if (iBad > 0)
    {
    OutputBadUTF8characters ();
    return;
    }

  // valid UTF8 sequence, add to line
  AddToLine ((const char *) m_UTF8Sequence, 0);
  m_phase = NONE;

  }  // end of CMUSHclientDoc::Phase_UTF8
