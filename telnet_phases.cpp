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

void CMUSHclientDoc::Phase_IAC (unsigned char & c)
  {
  char * p;
  unsigned char new_c = 0;

  switch (c)
    {
    case EOR                 : 
      
          m_phase = NONE; 
          p = "EOR"; 
          if (m_bConvertGAtoNewline)
            new_c = '\n';
          break;
    case SE                  : m_phase = NONE; p = "SE"; break;
    case NOP                 : m_phase = NONE; p = "NOP"; break;
    case DATA_MARK           : m_phase = NONE; p = "DM"; break;
    case BREAK               : m_phase = NONE; p = "BRK"; break;
    case INTERRUPT_PROCESS   : m_phase = NONE; p = "IP"; break;
    case ABORT_OUTPUT        : m_phase = NONE; p = "AO"; break;
    case ARE_YOU_THERE       : m_phase = NONE; p = "AYT"; break;
    case ERASE_CHARACTER     : m_phase = NONE; p = "EC"; break;
    case ERASE_LINE          : m_phase = NONE; p = "EL"; break;
    case GO_AHEAD            : 
          m_phase = NONE; 
          p = "GA"; 
          if (m_bConvertGAtoNewline)
            new_c = '\n';
          break;
    case SB                  : m_phase = HAVE_SUBNEGOTIATION; p = "SB"; break;
    case WILL                : m_phase = HAVE_WILL;           p = "WILL"; break;
    case WONT                : m_phase = HAVE_WONT;           p = "WONT"; break;
    case DO                  : m_phase = HAVE_DO;             p = "DO"; break;
    case DONT                : m_phase = HAVE_DONT;           p = "DONT"; break;
    default                  : m_phase = NONE;                p = "none"; break;
    } // end of switch
  TRACE1 ("<%s>", p);
  m_subnegotiation_type = 0;    // no subnegotiation type yet
  c = new_c;

  } // end of Phase_IAC

// WILL - we have IAC WILL x

void CMUSHclientDoc::Phase_WILL (const unsigned char c)
  {
// telnet negotiation : in response to WILL, we say DONT
// (except for compression, MXP, TERMINAL_TYPE and SGA), we *will* handle that)

  TRACE1 ("<%d>", c);
  m_phase = NONE;  // back to normal text after this character
  switch (c)
    {
    case TELOPT_COMPRESS:
    case TELOPT_COMPRESS2:
      // initialise compression library if not already decompressing
      if (!m_bCompressInitOK && !m_bCompress)
        m_bCompressInitOK = InitZlib (m_zCompress);
      if (m_bCompressInitOK && !m_bDisableCompression)
        {
        if (!m_CompressOutput)
          m_CompressOutput = (Bytef *) malloc (COMPRESS_BUFFER_LENGTH);
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
          TRACE1 ("\nSending IAC DO <%d>\n", c);
          unsigned char p [3] = { IAC, DO, c };
          SendPacket (p, sizeof p);
          if (c == TELOPT_COMPRESS2)
            m_bSupports_MCCP_2 = true;
          }
        else
          {   // not enough memory or already agreed to MCCP 2 - no compression
          TRACE1 ("\nSending IAC DONT <%d>\n", c);
          unsigned char p [3] = { IAC, DONT, c };
          SendPacket (p, sizeof p);
          }
        }   // end of compression wanted and zlib engine initialised
      else
        {
        TRACE1 ("\nSending IAC DONT <%d>\n", c);
        unsigned char p [3] = { IAC, DONT, c };
        SendPacket (p, sizeof p);
        }
      break;    // end of TELOPT_COMPRESS

    // here for SGA (Suppress GoAhead) 
    case SGA:
          {
          unsigned char p [3] = { IAC, DO, c };
          SendPacket (p, sizeof p);
          }
          break;  // end of SGA 

    // here for TELOPT_MUD_SPECIFIC 
    case TELOPT_MUD_SPECIFIC:
          {
          unsigned char p [3] = { IAC, DO, c };
          SendPacket (p, sizeof p);
          m_bIncoming_MUD_specific = true;
          }
          break;  // end of TELOPT_MUD_SPECIFIC 

    case TELOPT_ECHO:
        if (!m_bNoEchoOff)
            {
//            unsigned char p [3] = { IAC, DO, c };
//            SendPacket (p, sizeof p);
            m_bNoEcho = true;
            TRACE ("Echo turned off\n");
            }
          break; // end of TELOPT_ECHO

    case TELOPT_MXP:
          {
          unsigned char agree [3] = { IAC, DO, c };
          unsigned char disagree [3] = { IAC, DONT, c };
          if (m_iUseMXP == eNoMXP)
            {
            TRACE1 ("\nSending IAC DONT <%d>\n", c);
            SendPacket (disagree, sizeof disagree);
            }     // end of no MXP wanted
          else
            {
            TRACE1 ("\nSending IAC DO <%d>\n", c);
            SendPacket (agree, sizeof agree);
            if (m_iUseMXP == eQueryMXP)     // turn MXP on now
              MXP_On ();
            } // end of MXP wanted
          }
          break;  // end of MXP
          
    // here for EOR (End of record)
    case WILL_END_OF_RECORD:
          {
          if (m_bConvertGAtoNewline)
            {
            unsigned char p [3] = { IAC, DO, c };
            SendPacket (p, sizeof p);
            }   // we will handle it
          else
            {
            unsigned char p [3] = { IAC, DONT, c };
            SendPacket (p, sizeof p);
            }   // we won't
          }
          break;  // end of WILL_END_OF_RECORD


    default:
        {
        unsigned char p [3] = { IAC, DONT, c };
        SendPacket (p, sizeof p);
        }
        break;  // end of others

    } // end of switch

  } // end of Phase_WILL

void CMUSHclientDoc::Phase_WONT (const unsigned char c)
  {
// telnet negotiation : in response to WONT, we say DONT

  TRACE1 ("<%d>", c);
  m_phase = NONE;

  switch (c)
    {
    case TELOPT_ECHO:
        if (!m_bNoEchoOff)
          {
//          unsigned char p [3] = { IAC, DONT, c };
//          SendPacket (p, sizeof p);
          m_bNoEcho = false;
          TRACE ("Echo turned on\n");
          }
          break; // end of TELOPT_ECHO

    default:
      {
      unsigned char p [3] = { IAC, DONT, c };
      SendPacket (p, sizeof p);
      }
      break;
    } // end of switch
  } // end of Phase_WONT

void CMUSHclientDoc::Phase_DO (const unsigned char c)
  {
// telnet negotiation : in response to DO, we say WONT  
//  (except for SGA, echo, NAWS and Terminal type)

  TRACE1 ("<%d>", c);
  m_phase = NONE;
  switch (c)
    {

// if we are already in a mode do not agree again - see RFC 854 
// and forum subject 3061                           

    case SGA:

      if (!m_bSent_WILL_SGA)
        {
        unsigned char p [3] = { IAC, WILL, c };
        SendPacket (p, sizeof p);
        m_bSent_WILL_SGA = true;
        }
      break; // end of SGA 
                
    case TELOPT_MUD_SPECIFIC:

      if (!m_bSent_WILL_TELOPT_MUD_SPECIFIC)
        {
        unsigned char p [3] = { IAC, WILL, c };
        SendPacket (p, sizeof p);
        m_bSent_WILL_TELOPT_MUD_SPECIFIC = true;
        m_bOutgoing_MUD_specific = true;
        }
      break; // end of TELOPT_MUD_SPECIFIC 

    case TELOPT_TERMINAL_TYPE:   

      if (!m_bSent_WILL_TELOPT_TERMINAL_TYPE)
        {
        unsigned char p [3] = { IAC, WILL, c };
        SendPacket (p, sizeof p);
        m_bSent_WILL_TELOPT_TERMINAL_TYPE = true;
        }
      break; // end of TELOPT_TERMINAL_TYPE 

    case TELOPT_ECHO:
      if (!m_bSent_WILL_TELOPT_ECHO)
        {
        unsigned char p [3] = { IAC, WILL, c };
        SendPacket (p, sizeof p);
        m_bSent_WILL_TELOPT_ECHO = true;
        }
      break;  // end of TELOPT_ECHO

    case TELOPT_NAWS:
      {
      // option off - must be server initiated
      if (!m_bNAWS)
        {
        unsigned char p [3] = { IAC, WILL, TELOPT_NAWS };
        SendPacket (p, sizeof p);
        }
      m_bNAWS_wanted = true;
      SendWindowSizes (m_nWrapColumn);
      }
      break;

    case TELOPT_CHARSET:
      {
      unsigned char p [3] = { IAC, WILL, TELOPT_CHARSET };
      SendPacket (p, sizeof p);
      m_bCHARSET_wanted = true;
      }
      break;

    case TELOPT_MXP:
          {
          unsigned char agree [3] = { IAC, WILL, c };
          unsigned char disagree [3] = { IAC, WONT, c };
          if (m_iUseMXP == eNoMXP)
            {
            TRACE1 ("\nSending IAC WONT <%d>\n", c);
            SendPacket (disagree, sizeof disagree);
            }     // end of no MXP wanted
          else
            {
            TRACE1 ("\nSending IAC WILL <%d>\n", c);
            SendPacket (agree, sizeof agree);
            if (m_iUseMXP == eQueryMXP)     // turn MXP on now
              MXP_On ();
            } // end of MXP wanted
          }
          break;  // end of MXP

    default:
      {
      unsigned char p [3] = { IAC, WONT, c };
      SendPacket (p, sizeof p);
      }
      break;    // end of others
    }   // end of switch

  } // end of Phase_DO

void CMUSHclientDoc::Phase_DONT (const unsigned char c)
  {
// telnet negotiation : in response to DONT, we say WONT

  TRACE1 ("<%d>", c);
  m_phase = NONE;
  unsigned char p [3] = { IAC, WONT, c };
  SendPacket (p, sizeof p);

  switch (c)
    {

    case TELOPT_MXP:
          if (m_bMXP)
            MXP_Off (true);
          break;  // end of MXP

    }   // end of switch

  } // end of Phase_DONT

// SUBNEGOTIATION - we have IAC SB c

void CMUSHclientDoc::Phase_SUBNEGOTIATION (const unsigned char c)
  {
  if (c == SE)        // end of subnegotiation
    {
    TRACE ("<SE>");
    m_phase = NONE;
    }
  else if (c == IAC && m_subnegotiation_type == TELOPT_COMPRESS2)
    {
    TRACE ("<IAC>");
    m_phase = HAVE_COMPRESSION;
    m_iMCCP_type = 2;
    }
  else if (c == IAC && m_subnegotiation_type == TELOPT_MXP)
    {
    TRACE ("<IAC>");
    m_phase = HAVE_MXP;
    }
  else if (c == WILL && m_subnegotiation_type == TELOPT_COMPRESS)
    {
    TRACE ("<WILL>");
    m_phase = HAVE_COMPRESSION;
    m_iMCCP_type = 1;
    }
  // server requests: IAC SB TERMINAL-TYPE SEND IAC SE
  else if (c == 1 && m_subnegotiation_type == TELOPT_TERMINAL_TYPE)
    {
    TRACE ("<SEND>");
    // we reply: IAC SB TERMINAL-TYPE IS ... IAC SE
    // see: RFC 930 and RFC 1060
    unsigned char p1 [] = { IAC, SB, TELOPT_TERMINAL_TYPE, 0 }; 
    unsigned char p2 [] = { IAC, SE }; 
    unsigned char sResponse [40];
    int iLength = 0;

    // build up response, eg. IAC, SB, TELOPT_TERMINAL_TYPE, 0, "MUSHCLIENT", IAC, SE 

    // preamble
    memcpy (sResponse, p1, sizeof p1);
    iLength += sizeof p1;

    // ensure max of 20 so we don't overflow the field
    CString strTemp = m_strTerminalIdentification.Left (20);

    memcpy (&sResponse [iLength], strTemp, strTemp.GetLength ());
    iLength += strTemp.GetLength ();

    // postamble
    memcpy (&sResponse [iLength], p2, sizeof p2);
    iLength += sizeof p2;

    SendPacket (sResponse, iLength);
    }
  // server requests: IAC SB CHARSET REQUEST DELIMITER <name> DELIMITER
  else if (c == 1 && m_subnegotiation_type == TELOPT_CHARSET)       // 1 = REQUEST
    {
    m_phase = HAVE_CHARSET_REQUEST;  // now we need the delimiter
    m_charset_delimiter = 0;
    }
  else if (c == IAC)
    {
    TRACE ("<IAC>");
    m_phase = HAVE_IAC; // starting another IAC
    }
  else
    {
    TRACE1 ("<%d>", c);
    m_subnegotiation_type = c;    // remember type of subnegotiation

    if (m_subnegotiation_type == TELOPT_MUD_SPECIFIC)
      {
      m_phase = HAVE_MUD_SPECIFIC;
      m_strLast_MUD_specific_stuff_received.Empty ();
      }
    }

  } // end of Phase_SUBNEGOTIATION

// COMPRESSION - we have IAC SB COMPRESS IAC/WILL x

// we will return one of:
//  0 - error in starting compression - close world and display strMessage
//  1 - got IAC or unexpected input, do nothing
//  2 - compression OK - prepare for it

int CMUSHclientDoc::Phase_COMPRESSION (const unsigned char c, CString & strMessage)
  {
  if (c == SE)        // end of subnegotiation
    {        
    TRACE ("<SE>");
    // initialise compression library
    if (!m_bCompressInitOK && !m_bCompress)
      m_bCompressInitOK = InitZlib (m_zCompress);

    if (!(m_bCompressInitOK && m_CompressOutput && m_CompressInput))
      {
      strMessage = Translate ("Cannot process compressed output. World closed.");
      return 0;
      }

    m_bCompress = true;
    TRACE ("Compression on\n");

    int izError;
    if ((izError = inflateReset (&m_zCompress)) != Z_OK)
      {
      if (m_zCompress.msg)
        strMessage = TFormat ("Could not reset zlib decompression engine: %s",
                                 m_zCompress.msg);
      else
        strMessage = TFormat ("Could not reset zlib decompression engine: %i",
                                 izError);
      return 0;
      }   // end of bad engine reset
    m_phase = NONE;
    return 2;
    }   // end of IAC SB COMPRESS WILL/IAC SE
  m_phase = NONE;
  return 1;
  } // end of Phase_COMPRESSION

void CMUSHclientDoc::Phase_MXP (const unsigned char c)
  {
  if (c == SE)        // end of subnegotiation
    {        
    if (m_iUseMXP == eOnCommandMXP)   // if wanted now
      MXP_On ();
    }   // end of IAC SB MXP IAC SE
  m_phase = NONE;
  }  // end of Phase_MXP


void CMUSHclientDoc::Phase_CHARSET_REQUEST (const unsigned char c)
  {
  m_phase = HAVE_CHARSET;  // now we need the name
  m_charset_delimiter = c;
  m_charset_name.Empty ();  // no name yet
  }


void CMUSHclientDoc::Phase_MUD_SPECIFIC (const unsigned char c)
  {
  if (c == IAC ||   // IAC terminates
      c == 0 ||     // can't handle NUL
      m_strLast_MUD_specific_stuff_received.GetLength () > 127)  // bail out at 127 characters
    {
    m_phase = HAVE_IAC;

    // tell each plugin what we have received
    for (POSITION pluginpos = m_PluginList.GetHeadPosition(); pluginpos; )
      {
      CPlugin * pPlugin = m_PluginList.GetNext (pluginpos);


      if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
        continue;

      CString strReceived (m_strLast_MUD_specific_stuff_received);

      // see what the plugin makes of this,
      pPlugin->ExecutePluginScript (ON_PLUGIN_TELNET_OPTION,
                                    strReceived,  // what we got
                                    pPlugin->m_dispid_plugin_telnet_option); 

      }   // end of doing each plugin
    return;
    }

  m_strLast_MUD_specific_stuff_received += c;
  TRACE1 ("<%d>", c);
  }


void CMUSHclientDoc::Phase_CHARSET (const unsigned char c)
  {

  if (c == IAC || m_charset_name.GetLength () > 100)  // bail out at 100 characters
    {
    vector <string> v;
    // I changed the delimiter to IAC
    StringToVector ((const char *) m_charset_name, v, string (1, (char) IAC), false);

    bool found = false;
    CString strCharset = "US-ASCII"; // default

    if (m_font)
      {
//      LOGFONT lf;
//      m_font [0]->GetLogFont (&lf);

//      switch (lf.lfCharSet)
//        {
//        case ANSI_CHARSET: strCharset = "US-ASCII"; break;
//
//
//        } // end of switch

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

    m_phase = HAVE_IAC;
    return;
    }   // end of negotiation (eg, got IAC)

  // don't want 0x00 in our string, in case they are silly enough to use that
  if (c == m_charset_delimiter)
    m_charset_name += (char) IAC;    // delimiter becomes IAC
  else 
    m_charset_name += c;             // append to wanted name

  }
