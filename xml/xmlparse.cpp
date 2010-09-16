// XMLparse.cpp : implementation of the CXmltestDoc class
//
// BTW - CDATA sections look like this:

//  <![CDATA[ ... ]]> 

#include "stdafx.h"
#include "..\MUSHclient.h"
#include "..\doc.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CRLF "\r\n"

UINT iLineLastItemFound;


CString CXMLparser::Get_XML_Entity (CString & strName)
  {

CString strEntityContents;

  // look for &#nnn; 

  if (strName [0] == '#')
    {
    int iResult = 0;

    // validate and work out number
    if (strName [1] == 'x')
      {
      for (int i = 2; i < strName.GetLength (); i++)
        {
        if (!isxdigit (strName [i]))
          {
          ThrowErrorException ("Invalid hex number in XML entity: &%s;" ,
                              (LPCTSTR) strName);
          return "";
          }

        int iNewDigit = toupper (strName [i]);
        if (iNewDigit >= 'A')
          iNewDigit -= 7;
        if (iResult & 0xF0)
          ThrowErrorException ("Invalid hex number in XML entity: &%s; "
                               "- maximum of 2 hex digits",
                              (LPCTSTR) strName);
          iResult = (iResult << 4) + iNewDigit - '0';
        }
      } // end of hex entity
    else
      {
      for (int i = 1; i < strName.GetLength (); i++)
        {
        if (!isdigit (strName [i]))
          {
          ThrowErrorException ("Invalid number in XML entity: &%s;" ,
                              (LPCTSTR) strName);
          return "";
          }
        iResult *= 10;
        iResult += strName [i] - '0';
        }
      } // end of decimal entity

    if (iResult != 9)       // we will accept tabs ;)
      if (iResult < 32 ||   // don't allow nonprintable characters
          iResult > 255)   // don't allow characters more than 1 byte
          {
          ThrowErrorException ("Disallowed number in XML entity: &%s;" ,
                              (LPCTSTR) strName);
          return "";
          }
    unsigned char cOneCharacterLine [2] = { (unsigned char) iResult, 0};
    return (char *) cOneCharacterLine;
    } // end of entity starting with #

  
  // look up global entities first
  if (App.m_EntityMap.Lookup (strName, strEntityContents))
    return strEntityContents;
  // then try ones for this document
  else if (m_CustomEntityMap.Lookup (strName, strEntityContents))
    return strEntityContents;

  ThrowErrorException ("Unknown XML entity: &%s;" ,
                        (LPCTSTR) strName);

  return "";    // to avoid warning
  }  // end of CXMLparser::Get_XML_Entity




void CXMLparser::SkipSpaces (void)
  {

  // bypass whitespace
  while (isspace ((unsigned char) m_xmlBuff [m_xmlPos]))
    {
    if (m_xmlBuff [m_xmlPos] == '\n')
      m_xmlLine++;   // count lines
    m_xmlPos++;
    }   // end of whitespaces loop

  } // end of CXMLparser::SkipSpaces


void CXMLparser::SkipComments (const bool bAndSpaces)
  {
  while (true)  // loop in case consecutive comments, or comment - space - comment
    {

    if (bAndSpaces)
      SkipSpaces ();

    // comment?

    if (m_xmlBuff [m_xmlPos] == '<' && 
        m_xmlBuff [m_xmlPos + 1] == '!' && 
        m_xmlBuff [m_xmlPos + 2] == '-' && 
        m_xmlBuff [m_xmlPos + 3] == '-')
      {
      // skip comment

      m_xmlPos += 4; // skip <!--

      while (m_xmlBuff [m_xmlPos] != '-' || 
             m_xmlBuff [m_xmlPos + 1] != '-' || 
             m_xmlBuff [m_xmlPos + 2] != '>')
        {
        if (m_xmlBuff [m_xmlPos] == 0)
          ThrowErrorException ("Unterminated comment at end of file");

        if (m_xmlBuff [m_xmlPos] == '-' && 
            m_xmlBuff [m_xmlPos + 1] == '-')
          ThrowErrorException ("Invalid '--' inside comment");

        if (m_xmlBuff [m_xmlPos] == '\n')
          m_xmlLine++;  // count lines
        m_xmlPos++;
        }
      
      // here for end of comment

      m_xmlPos += 3; // skip -->

      } // end of comment
    else
      return; // no comment, all done

    };

  } // end of SkipComments

void CXMLparser::BuildStructure (CFile * file)
  {

  m_xmlLength = file->GetLength ();

  // can hardly be XML if empty file
  if (m_xmlLength == 0)
    ThrowErrorException ("Zero length XML file \"%s\"",
                          (LPCTSTR) file->GetFilePath ());

  // sanity check - don't read 20 Mb files into memory
  if (m_xmlLength > MAX_XML_DOCUMENT_SIZE)
    ThrowErrorException ("XML file \"%s\"too large - maximum permitted is %i bytes", 
                          (LPCTSTR) file->GetFilePath (),
                          MAX_XML_DOCUMENT_SIZE);

  char * p = m_strxmlBuffer.GetBuffer (m_xmlLength + 1);

  iLineLastItemFound = 0;

  if (file->Read (p, m_xmlLength) != m_xmlLength)
    {
    m_strxmlBuffer.ReleaseBuffer (0);
    ThrowErrorException ("Could not read XML file");
    }

  m_strxmlBuffer.ReleaseBuffer (m_xmlLength);

  // look for Unicode  (FF FE)
  if ((unsigned char) m_strxmlBuffer [0] == 0xFF &&
      (unsigned char) m_strxmlBuffer [1] == 0xFE)
    {
    CString buf2;
    const char * q =  m_strxmlBuffer;
    q += 2; // skip indicator bytes
    // find required buffer length
    int length = WideCharToMultiByte (CP_UTF8, 0, (LPCWSTR) q, (m_xmlLength - 2) / 2, 
                    NULL, 0, NULL, NULL);
    // make a new string with enough length to hold it
    char * p = buf2.GetBuffer (length);
    // convert it
    WideCharToMultiByte (CP_UTF8, 0, (LPCWSTR) q, (m_xmlLength - 2) / 2, 
              p, length, NULL, NULL);
    buf2.ReleaseBuffer (length);
    // copy to our buffer
    m_strxmlBuffer = buf2;
    }
  else 
    // look for UTF-8 indicator bytes  (EF BB BF)
    if ((unsigned char) m_strxmlBuffer [0] == 0xEF &&
        (unsigned char) m_strxmlBuffer [1] == 0xBB &&
        (unsigned char) m_strxmlBuffer [2] == 0xBF)
      m_strxmlBuffer = m_strxmlBuffer.Mid (3);  // skip them


  // convert tabs to spaces, we don't want tabs in our data
  m_strxmlBuffer.Replace ('\t', ' ');

  SeeIfBase64 (m_strxmlBuffer);

  m_xmlBuff = m_strxmlBuffer;    // get const char * pointer to buffer

  ProcessNode (m_xmlRoot);  // process root node

  }   // end of CXMLparser::BuildStructure

/*

  Process all siblings in a node

  Special cases: 

  <?xml version="1.0" ?>
  <!DOCTYPE blah >

  Other expected cases:
   
  <node> ... contents ... </node>
  <node a='1' b='2'> ... contents </node>
  <node />
  <node a='1' b='2' />

*/

CString CXMLparser::GetName (const char * sType)
  {
int iStart;
int iNameLen;

  iStart = m_xmlPos; // start of name

  if (m_xmlBuff [m_xmlPos] == 0)
    ThrowErrorException ("Unexpected end-of-file looking for %s name",
                          sType);

  // name must start with letter or underscore
  if (!isalpha (m_xmlBuff [m_xmlPos]) && m_xmlBuff [m_xmlPos]  != '_')
      ThrowErrorException ("%s name must start with letter or underscore, "
                           "but starts with \"%c\"",
                          sType,
                          m_xmlBuff [m_xmlPos]);

  // build up name

  while (isalnum (m_xmlBuff [m_xmlPos]) ||
         m_xmlBuff [m_xmlPos] == '-' ||
         m_xmlBuff [m_xmlPos] == '.' ||
         m_xmlBuff [m_xmlPos] == '_')
         m_xmlPos++;

  // OK - we now have a name, unless it is empty 

  iNameLen = m_xmlPos - iStart;
  if (iNameLen == 0)
    // name starting with > is really an omitted name
    if (m_xmlBuff [m_xmlPos] == '>')
      ThrowErrorException ("%s name not supplied",
                          sType);
    // detect a comment inside an element as a special case
    else
    if (m_xmlBuff [m_xmlPos] == '<' &&
        m_xmlBuff [m_xmlPos + 1] == '!' &&
        m_xmlBuff [m_xmlPos + 2] == '-' &&
        m_xmlBuff [m_xmlPos + 3] == '-')
      ThrowErrorException ("Comment not valid here");
    else
      ThrowErrorException ("%s name cannot start with '%c'",
                          sType,
                          m_xmlBuff [m_xmlPos]);

  // sanity check - make sure name not ridiculous
  if (iNameLen > MAX_XML_NAME_LENGTH)
    ThrowErrorException ("%s name is too long (%i characters) "
                         " maximum permitted is %i characters",
        sType,
        iNameLen,
        MAX_XML_NAME_LENGTH);

  // in case spaces before next thing
  SkipSpaces ();

  // construct name from buffer
  return CString (&m_xmlBuff [iStart], iNameLen);

  } // end of CXMLparser::GetName

CString CXMLparser::GetValue (const char * sType, const char * sName)
  {
char cQuote;
UINT iStart;
CString strValue;
int iContentLen;

  // in case spaces before the value
  SkipSpaces ();

  // find what sort of quote they are using
  cQuote = m_xmlBuff [m_xmlPos];

  if (cQuote == 0)
    ThrowErrorException ("Unexpected end-of-file looking for value of %s named %s",
                          sType,
                          sName);

  if (cQuote != '\'' && cQuote != '\"')
    // value starting with > is really an omitted value
    if (cQuote == '>')
      ThrowErrorException ("Value for %s not supplied",
                          sType);

    // detect a comment here as a special case
    else
    if (m_xmlBuff [m_xmlPos] == '<' &&
        m_xmlBuff [m_xmlPos + 1] == '!' &&
        m_xmlBuff [m_xmlPos + 2] == '-' &&
        m_xmlBuff [m_xmlPos + 3] == '-')
      ThrowErrorException ("Comment not valid here");
    else
      ThrowErrorException ("Value for %s named '%s' not supplied (in quotes)",
                            sType,
                            sName);

  m_xmlPos++; // get past quote

  iStart = m_xmlPos; // start of value

  while (m_xmlBuff [m_xmlPos] != cQuote)
    {
    if (m_xmlBuff [m_xmlPos] == '\n' || 
        m_xmlBuff [m_xmlPos] == '\r' ||
        m_xmlBuff [m_xmlPos] == 0)
      ThrowErrorException ("Value for %s named '%s' not terminated (no closing quote)",
                            sType,
                            sName);

    // <![CDATA[ terminator invalid here (ie. ]]> )
    if (m_xmlBuff [m_xmlPos] == ']' &&
           m_xmlBuff [m_xmlPos + 1] == ']' &&
           m_xmlBuff [m_xmlPos + 2] == '>')
      ThrowErrorException ("Value for %s named '%s' may not contain CDATA terminator: ']]>'",
                            sType,
                            sName);

    // don't let them slip in non-printables 
    if ((unsigned char) m_xmlBuff [m_xmlPos] < ' ')
      ThrowErrorException ("Non-printable character, code %i, not permitted for "
                           "value for %s named '%s'",
                            (int) m_xmlBuff [m_xmlPos], 
                            sType,
                            sName);

    m_xmlPos++; 
    }

  iContentLen = m_xmlPos - iStart;

  m_xmlPos++;   // get past quote

  SkipSpaces ();

  // construct value from buffer
  return CString (&m_xmlBuff [iStart], iContentLen);

  } // end of CXMLparser::GetValue 

void CXMLparser::ProcessNode (CXMLelement & parent)
  {

CString strName;
CString strValue;
CString strPrefix;
UINT iNodeStart = m_xmlPos;
UINT iFirstLine = m_xmlLine;

  while (m_xmlBuff [m_xmlPos])
    {

    // bypass spaces and comments

    SkipComments (true);

    // nothing left? all done

    if (m_xmlBuff [m_xmlPos] == 0)
      break;

    // look for element start

    if (m_xmlBuff [m_xmlPos] != '<')
      {
      // not element - must be content

      // can't have content at root level
      if (parent.strName.IsEmpty ())
        ThrowErrorException ("Expected '<', got \"%c\" (content not permitted here)",
            m_xmlBuff [m_xmlPos]);

      if (m_xmlBuff [m_xmlPos] == '\n')
        m_xmlLine++;   // count lines

      // <![CDATA[ terminator invalid here (ie. ]]> )
      if (m_xmlBuff [m_xmlPos] == ']' &&
             m_xmlBuff [m_xmlPos + 1] == ']' &&
             m_xmlBuff [m_xmlPos + 2] == '>')
        ThrowErrorException ("CDATA terminator ']]>' invalid");

      // don't let them slip in non-printables 
      if ((unsigned char) m_xmlBuff [m_xmlPos] < ' ' &&
          (unsigned char) m_xmlBuff [m_xmlPos] != '\n' && 
          (unsigned char) m_xmlBuff [m_xmlPos] != '\r')
        ThrowErrorException ("Non-printable character, code %i, not permitted",
                              (int) m_xmlBuff [m_xmlPos]);

      m_xmlPos++;  // skip character
      continue;     // keep processing content

      }

    m_xmlPos++;  // skip <

    // special cases: <?xml blah?> <!doctype blah> </end-of-something>

    if (m_xmlBuff [m_xmlPos] == '!' || 
        m_xmlBuff [m_xmlPos] == '?' ||
        m_xmlBuff [m_xmlPos] == '/')
      {
      strPrefix = m_xmlBuff [m_xmlPos]; 
      m_xmlPos++;    // bypass special types
      }
    else
      strPrefix.Empty ();

    // special and bizarre case of <![CDATA[ ... ]]>
    if (strPrefix == '!' &&
        m_xmlBuff [m_xmlPos] == '[' &&
        m_xmlBuff [m_xmlPos + 1] == 'C' &&
        m_xmlBuff [m_xmlPos + 2] == 'D' &&
        m_xmlBuff [m_xmlPos + 3] == 'A' &&
        m_xmlBuff [m_xmlPos + 4] == 'T' &&
        m_xmlBuff [m_xmlPos + 5] == 'A' &&
        m_xmlBuff [m_xmlPos + 6] == '[')
      {

      // can't have content at root level
      if (parent.strName.IsEmpty ())
        ThrowErrorException ("<![CDATA[ invalid at root level");

      m_xmlPos += 7;  // skip [CDATA[

      while (m_xmlBuff [m_xmlPos] != ']' ||
             m_xmlBuff [m_xmlPos + 1] != ']' ||
             m_xmlBuff [m_xmlPos + 2] != '>')
        {

        // shouldn't have end-of-file before the ]]>
        if (m_xmlBuff [m_xmlPos] == 0)
          ThrowErrorException ("Unexpected end-of-file inside [CDATA[ definition");

        // don't let them slip in non-printables 
        if ((unsigned char) m_xmlBuff [m_xmlPos] < ' ' &&
            (unsigned char) m_xmlBuff [m_xmlPos] != '\n' && 
            (unsigned char) m_xmlBuff [m_xmlPos] != '\r')
          ThrowErrorException ("Non-printable character, code %i, not permitted",
                                (int) m_xmlBuff [m_xmlPos]);

        if (m_xmlBuff [m_xmlPos] == '\n')
          m_xmlLine++;   // count lines
        m_xmlPos++;   // try next character
        }

      m_xmlPos += 3;  // skip ]]>
      continue; // back to start of loop

      } // end of <![CDATA[ section

    int iLine = m_xmlLine;    // line where we expect to see name
    strName = GetName ("Element");

    // check for sequence </blah xxx>
    if (strPrefix == '/' && m_xmlBuff [m_xmlPos] != '>')
      if (m_xmlBuff [m_xmlPos] == 0)
        ThrowErrorException ("Unexpected end-of-file after '</%s'",
            (LPCTSTR) strName);
      else
        ThrowErrorException ("Surplus characters after '</%s'",
            (LPCTSTR) strName);

    // check for sequence </blah>

    if (strPrefix == '/')
      {
      if (strName != parent.strName)
        if (parent.strName.IsEmpty ())  // can't go up a level here :)
          ThrowErrorException ("Unexpected closing tag </%s> - "
                               "no corresponding opening tag",
                                (LPCTSTR) strName);

        else
          ThrowErrorException ("Elements terminated out of sequence, "
                               "expected </%s>, got </%s>",
                                (LPCTSTR) parent.strName,
                                (LPCTSTR) strName);

      AssembleContent (parent, iFirstLine, iNodeStart);

      m_xmlPos++; // skip terminating >

      return;    // terminated this node
      }

    if (strPrefix == '!')
      ProcessDeclaration (strName);   // process declaration, eg. <!DOCTYPE
    else
      {     // not a declaration, must be an ordinary element

      // time to create a new element
      CXMLelement * pElement = new CXMLelement;

      if (!pElement)
        ThrowErrorException ("Could not allocate memory for XML parsing of element %s",
                              (LPCTSTR) strName);

      // remember its name
      pElement->strName = strPrefix + strName;
      pElement->iLine = iLine;

      parent.ChildrenList.AddTail (pElement);    // add to parent's list of children

      // OK, we have a new sibling for our parent, let's see if it has any attributes ...

      ProcessAttributes (*pElement);  // process its attributes

      // hmm - we have now got something like <blah> ...
      // drop down to find children for it - unless 'empty' element

      if (!pElement->bEmpty)
        ProcessNode (*pElement);

      }   // end of non-declaration
    } // end of processing buffer

  if (!parent.strName.IsEmpty ())
    ThrowErrorException ("Unexpected end-of-file while looking for </%s>",
                          (LPCTSTR) parent.strName);

  }   // end of CXMLparser::ProcessNode 

void CXMLparser::AssembleContent (CXMLelement & node, 
                                    const UINT iFirstLine, 
                                    const UINT iNodeStart)
  {

/*
   work out content - it will be non-nested text belonging to this element
   eg. <a>xxx<b>yyy</b>zzz</a>
      "a" will have content xxxzzz
      "b" will have content yyy
   whitespace is preserved

   You can reassemble an entire element (including nested ones) by
   concatenating the contents of its children.
  
   For cross-platform consistency, carriage returns are dropped,
    and linefeeds become carriage-return/linefeed

  */

int iContentLen = m_xmlPos - iNodeStart;

int iLines = m_xmlLine - iFirstLine;    // lines of content
const char * pi = &m_xmlBuff [iNodeStart];    // copy from
const char * pl = &m_xmlBuff [m_xmlPos];      // limit of copy
char * po = node.strContent.GetBuffer (iContentLen + 1 + iLines); // copy to
int iDepth = 0;
bool bInside = false;
char cQuote = ' ';

  while (pi < pl)   // copy, dropping nested elements
    {

    // they might have <blah value=">">
    // so we have to skip special characters inside quotes

    if (bInside && (*pi == '\'' || *pi == '\"'))
      {
      cQuote = *pi;   // for terminator
      pi++;
      while (pi < pl && *pi != cQuote)
        pi++;
      pi++;  // skip terminating quote
      continue;   // back to start of loop
      } // end of quotes inside a definition

    // <![CDATA[  is special - just copy contents

    if (pi [0] == '<' &&
        pi [1] == '!' &&
        pi [2] == '[' &&
        pi [3] == 'C' &&
        pi [4] == 'D' &&
        pi [5] == 'A' &&
        pi [6] == 'T' &&
        pi [7] == 'A' &&
        pi [8] == '[')
      {

      pi += 9;  // skip <![CDATA[

      // this shouldn't happen
      if (bInside)
        ThrowErrorException ("Unexpected '<![CDATA[' inside element definition");

      // should be terminated as we checked earlier, but may as well check again
      while (pi < pl &&
             (pi [0] != ']' ||
              pi [1] != ']' ||
              pi [2] != '>'))
        {
       // copy if not nested, and not inside an element definition
       //  -- omit carriage returns
       if (iDepth == 0 && 
           *pi != '\r')
         {
         // make linefeeds into carriage return/linefeed
         if (*pi == '\n')
           *po++ = '\r';
         // special treatment for ampersands, so they don't get converted
         // into entities ...
         if (*pi == '&')
           *po++ = '\x01';
         else
          *po++ = *pi;    // copy it
         }
        pi++;
        }  // end of inside <![CDATA[

      pi += 3;  // skip ]]>
      continue; // back to start of loop

      } // end of <![CDATA[ section

    if (*pi == '<')   // going into an element
      {
      bInside = true;
      if (pi [1] == '/')   // </tag> drops a level
        {
        iDepth--;
       if (iDepth < 0)
         iDepth = 0;    // cannot have negative depth
        }
      else
        if (pi [1] != '!' && pi [1] != '?')  // excepting special ones
          iDepth++;           // <tag> goes up a level
      }

   // copy if not nested, and not inside an element definition
   //  -- omit carriage returns
   if (iDepth == 0 && 
      !bInside && 
      *pi != '\r')
     {
     // make linefeeds into carriage return/linefeed
     if (*pi == '\n')
       *po++ = '\r';
     *po++ = *pi;    // copy if not inside an element
     }
    
    // leaving an element (only occurs 'inside' a <... sequence )
   if (bInside && pi [0] == '/' && pi [1] == '>') 
     {
     iDepth--;  // cancel level gain - this tag is empty
     if (iDepth < 0)
       iDepth = 0;    // cannot have negative depth
     }
    else
      if (*pi == '>')   // leaving an element
        bInside = false;

    pi++; // onto next

    }
  
  *po = 0;  // terminating null
  node.strContent.ReleaseBuffer (-1);

  // get rid of entities (such as &lt;) in the content

  node.strContent = ReplaceEntities (node.strContent);

  // now, in case we had <![CDATA[ sections, replace 0x01 by &

  node.strContent.Replace ('\x01', '&');

  } // end of CXMLparser::AssembleContent 

void CXMLparser::ProcessAttributes (CXMLelement & node)
  {
CAttribute * pAttribute;
CString strName;
CString strValue;

  while (m_xmlBuff [m_xmlPos] != '>')
    {
    // look for "empty element" terminator: />
    if (m_xmlBuff [m_xmlPos] == '/' && m_xmlBuff [m_xmlPos + 1] == '>')
      {
      node.bEmpty = true;
      m_xmlPos++; // skip '/' symbol
      break;
      }

    // look for terminator ?> at end of <?xml tag
    if (m_xmlBuff [m_xmlPos] == '?' && 
        m_xmlBuff [m_xmlPos + 1] == '>' &&
        node.strName [0] == '?'
        )
      {
      node.bEmpty = true;
      m_xmlPos++; // skip '?' symbol
      break;
      }

    // shouldn't have end-of-file before the >
    if (m_xmlBuff [m_xmlPos] == 0)
      ThrowErrorException ("Unexpected end-of-file inside element definition for %s",
                            (LPCTSTR) node.strName);

    // we really should have a name now ...

    strName = GetName ("Attribute");

    // already there? error
    if (node.AttributeMap.Lookup (strName, pAttribute))
      ThrowErrorException ("Duplicate attribute name '%s'",
                            (LPCTSTR) strName);

    // in case spaces before next thing
    SkipSpaces ();

    if (m_xmlBuff [m_xmlPos] != '=')
      ThrowErrorException ("Attribute name '%s' not followed by '='",
                            (LPCTSTR) strName);

    m_xmlPos++;    // skip the = sign

    int iLine =  m_xmlLine; // line that = sign is on

    strValue = GetValue ("attribute", strName);

    pAttribute = new CAttribute;

    if (!pAttribute)
      ThrowErrorException ("Could not allocate memory for XML parsing of attribute %s",
                            (LPCTSTR) strName);

    pAttribute->strName = strName;
    pAttribute->strValue = ReplaceEntities (strValue);
    pAttribute->iLine = iLine;

    // add to map
    node.AttributeMap.SetAt (strName, pAttribute);

    } // end of looking for attributes

  m_xmlPos++; // skip '>' symbol


  }   // end of CXMLparser::ProcessAttributes

void CXMLparser::ProcessDeclaration (const CString strDeclaration)
  {

  if (strDeclaration == "DOCTYPE")
    ProcessDoctype ();
  else
    ThrowErrorException ("Unrecognised declaration: <!%s ...>",
                         (LPCTSTR) strDeclaration);
  } // end of CXMLparser::ProcessDeclaration

void CXMLparser::ProcessDoctype (void)
  {

  m_strDocumentName = GetName ("Root document");

  // document name may be followed by SYSTEM or PUBLIC

  if (isalpha (m_xmlBuff [m_xmlPos]))
    {
    CString strName = GetName ("External ID");
    
    // we'll get and ignore these for future compatibility
    if (strName == "SYSTEM")
      GetValue ("SYSTEM", "DTD name");     // eg. SYSTEM "c:\blah\my.dtd"
    else
    if (strName == "PUBLIC")
      {
      GetValue ("PUBLIC", "Public ID");    // eg. PUBLIC "blah" "c:\blah\my.dtd"
      GetValue ("PUBLIC", "DTD name");
      }
    else
      ThrowErrorException ("Unsupported declaration \"%s\" inside <!DOCTYPE ...>",
                            (LPCTSTR) strName);


    } // end of SYSTEM/PUBLIC

  if (m_xmlBuff [m_xmlPos] == '[')
    ProcessDTD ();
  
  if (m_xmlBuff [m_xmlPos] != '>')
    {

    // shouldn't have end-of-file before the >
    if (m_xmlBuff [m_xmlPos] == 0)
      ThrowErrorException ("Unexpected end-of-file inside <!DOCTYPE> declaration");

    ThrowErrorException ("Unsupported declaration inside <!DOCTYPE ...>");

    } // end of looking for attributes

  m_xmlPos++; // skip '>' symbol


  }   // end of CXMLparser::ProcessDoctype

void CXMLparser::ProcessDTD (void)
  {
CString strName;

  m_xmlPos++;    // skip the [ symbol

  SkipComments (true);

  while (m_xmlBuff [m_xmlPos] != ']')
    {

    // shouldn't have end-of-file before the >
    if (m_xmlBuff [m_xmlPos] == 0)
      ThrowErrorException ("Unexpected end-of-file inside DTD declaration");

    // look for declaration start

    if (m_xmlBuff [m_xmlPos] != '<' || m_xmlBuff [m_xmlPos + 1] != '!')
      ThrowErrorException ("Expected '<!', got %c",
            m_xmlBuff [m_xmlPos]);

    m_xmlPos += 2;  // skip <!

    strName = GetName ("DTD Declaration");

    // we only support <!ENTITY ... > right now
    if (strName == "ENTITY")
      ProcessEntity ();
    else
      ThrowErrorException ("Unsupported declaration <!%s ...>",
                          (LPCTSTR) strName);

    } // end of looking for declarations

  m_xmlPos++; // skip ']' symbol

  SkipComments (true);

  } // end of CXMLparser::ProcessDTD

void CXMLparser::ProcessEntity (void)
  {
CString strName;
CString strValue;
CString strTemp;

  strName = GetName ("entity");

  strTemp = strName.Left (3);
  strTemp.MakeUpper ();

  if (strTemp == "XML")
    ThrowErrorException ("Names starting with \"XML\" are reserved");

  strValue = GetValue ("entity", strName);

  // entities might have entities in them, so replace them
  m_CustomEntityMap.SetAt (strName, ReplaceEntities (strValue));

  if (m_xmlBuff [m_xmlPos] != '>')
    ThrowErrorException ("Expected '>', got %c",
        m_xmlBuff [m_xmlPos]);

  m_xmlPos++;   // skip >

  // bypass spaces and comments

  SkipComments (true);

  } // end of CXMLparser::ProcessEntity 

CString CXMLparser::ReplaceEntities (const CString strSource)
    {

// look for entities imbedded in the string
const char * p = strSource;
const char * pStart = strSource;   // where buffer starts
const char * pEntity;
CString strFixedValue;
CString strEntity;
long length;

  // quick check to eliminate ones that don't have imbedded entities
  if (strSource.Find ('&') == -1)
    return strSource;

  strFixedValue.Empty ();

  for ( ; *p; p++)
    {
    if (*p == '&')
      {

      // copy up to ampersand
      length = p - pStart;
      
      if (length > 0)
        strFixedValue += CString (pStart, length);

      p++;    // skip ampersand
      pEntity = p; // where entity starts
      if (*p == '#')
        p++;
      while (isalnum (*p) ||
             *p == '-' ||
             *p == '.' ||
             *p == '_')
             p++;

      if (*p != ';')
        ThrowErrorException ("No closing \";\" in XML entity argument \"&%s\"", 
                  (LPCTSTR) CString (pEntity, p - pEntity)); 

      CString s (pEntity, p - pEntity);
      strFixedValue += Get_XML_Entity (s);    // add to list
  
      pStart = p + 1;   // move on past the entity

      } // end of having an ampersand 

    } // end of processing the value

  // copy last bit

  strFixedValue += pStart;

  return strFixedValue;
  }   // end of CXMLparser::ReplaceEntities

CAttribute * GetAttribute (CXMLelement & node,
                                       const char * sName)
  {
CAttribute * pAttribute;

  if (!node.AttributeMap.Lookup (sName, pAttribute))
    return NULL; // not there

  pAttribute->bUsed = true;   // note we used it

  // note line it was found on
  iLineLastItemFound = pAttribute->iLine;

  return pAttribute;
  } // end of GetAttribute


// get a boolean attribute - return true if found

bool Get_XML_boolean (CXMLelement & node,
                          const char * sName,
                          bool & bValue,
                          const bool bUseDefault)
  {
CString strValue;

  if (!Get_XML_string (node, sName, strValue, bUseDefault))
    if (bUseDefault)
      return false;

  // be nice and let them slip in Y or y
  strValue.MakeLower ();

  if (strValue == "0" ||
      strValue == "" ||   // empty can be considered false
      strValue == "no" ||
      strValue == "n" ||
      strValue == "false" ||
      strValue == "f"
      )
    {
    bValue = false;
    return true;
    }

  if (strValue == "1" ||
      strValue == "-1" ||
      strValue == "yes" ||
      strValue == "y" ||
      strValue == "true" ||
      strValue == "t"
      )
    {
    bValue = true;
    return true;
    }

  ThrowErrorException ("Boolean attribute named \"%s\" has unrecognised value \"%s\"",
                       sName,
                       (LPCTSTR) strValue);

  bValue = false;
  return true;
  }  // end of Get_XML_boolean
                   
bool Get_XML_boolean_ushort_mask (CXMLelement & node,
                          const char * sName,
                          unsigned short & bValue,
                          const unsigned short iMask,
                          const bool bUseDefault)
  {
bool b;

  if (!Get_XML_boolean (node, sName, b, bUseDefault))
    if (bUseDefault)
      return false;
  
  if (b)
    bValue |= iMask;    // if true, set appropriate bit
  else
    bValue &= ~iMask;   // otherwise clear it

  return true;
  }  // end of Get_XML_boolean_ushort_mask

bool Get_XML_boolean_int_mask (CXMLelement & node,
                          const char * sName,
                          int & bValue,
                          const int iMask,
                          const bool bUseDefault)
  {
bool b;

  if (!Get_XML_boolean (node, sName, b, bUseDefault))
    if (bUseDefault)
      return false;;
  
  if (b)
    bValue |= iMask;    // if true, set appropriate bit
  else
    bValue &= ~iMask;   // otherwise clear it

  return true;
  }  // end of Get_XML_boolean_int_mask

bool Get_XML_boolean_ushort (CXMLelement & node,
                          const char * sName,
                          unsigned short & bValue,
                          const bool bUseDefault)
  {
bool b;

  if (!Get_XML_boolean (node, sName, b, bUseDefault))
    if (bUseDefault)
      return false;

  bValue = b;

  return true;
  }  // end of Get_XML_boolean_ushort

bool Get_XML_boolean_BOOL (CXMLelement & node,
                          const char * sName,
                          BOOL & bValue,
                          const bool bUseDefault)
  {
bool b;

  if (!Get_XML_boolean (node, sName, b, bUseDefault))
    if (bUseDefault)
      return false;

  bValue = b;

  return true;
  }  // end of Get_XML_boolean_BOOL

bool Get_XML_number (CXMLelement & node,
                        const char * sName,
                        long & iValue, 
                        const bool bUseDefault,
                        long iMinimum,
                        long iMaximum)
  {

CString strValue;

  if (!Get_XML_string (node, sName, strValue, bUseDefault))
    if (bUseDefault)
      return false;
    else
      strValue = "0";   // default is zero

__int64 iResult = 0;
const char * p =  strValue;
bool bNegative = false;

  // see if sign given
  if (*p == '-')
    {
    bNegative = true;
    p++;
    }
  else
    if (*p == '+')
      p++;

  if (*p == 0)
      ThrowErrorException ("No number suppied for numeric attribute named '%s'" ,
                          sName);

  if (!isdigit (*p))
    ThrowErrorException ("Invalid number \"%s\" for numeric attribute named '%s'" ,
                          (LPCTSTR) strValue,
                          sName);

  for (iResult = 0; *p; p++)
    {
    if (!isdigit (*p))
      ThrowErrorException ("Invalid character '%c' in numeric attribute named '%s'" ,
                          *p,
                          sName);

    iResult *= 10;
    iResult += *p - '0';

    if (iResult > ((__int64) LONG_MAX + 1))   // to allow for LONG_MIN which will be one more
      ThrowErrorException ("Value '%s' too large in numeric attribute named '%s'" ,
                          (LPCTSTR) strValue,
                          sName);

    }

  // make negative if necessary
  if (bNegative)
    iResult = 0 - iResult;

  // fix up default 0, 0 to be 0, 1
  if (iMinimum == 0 && iMaximum == 0)
      iMaximum = 1;

  if (iResult > iMaximum)
    ThrowErrorException ("Value '%s' too large in numeric attribute named '%s'. "
                         "Range is %ld to %ld.",
                        (LPCTSTR) strValue,
                         sName,
                         iMinimum, 
                         iMaximum);

  if (iResult < iMinimum)
    ThrowErrorException ("Value '%s' too small in numeric attribute named '%s'. "
                         "Range is %ld to %ld.",
                        (LPCTSTR) strValue,
                         sName,
                         iMinimum, 
                         iMaximum);

  iValue = (long) iResult;
  return true;

  }  // end of Get_XML_number

bool Get_XML_double  (CXMLelement & node,
                        const char * sName,
                        double & dValue,
                        const bool bUseDefault,
                        const double dMinimum,
                        const double dMaximum)
  {
CString strValue;

  if (!Get_XML_string (node, sName, strValue, bUseDefault))
    if (bUseDefault)
      return false;
    else
      strValue = "0";   // default is zero

double dResult = 0;
const char * p =  strValue;
int iDots = 0;
int iExponents = 0;
   
  // see if sign given
  if (*p == '-' || *p == '+')
    p++;

  if (*p == 0)
      ThrowErrorException ("No number suppied for numeric attribute named '%s'" ,
                          sName);

  if (!isdigit (*p))
    ThrowErrorException ("Invalid number \"%s\" for numeric attribute named '%s'" ,
                          (LPCTSTR) strValue,
                          sName);

  for ( ; *p; p++)
    {
    if (*p == '.')
      {
      if (++iDots > 1)
        ThrowErrorException ("Too many decimal places for numeric attribute named '%s'" ,
                            sName);
      }  // end of decimal place
    else if (toupper (*p) == 'E')
      {
      if (++iExponents > 1)
        ThrowErrorException ("Too many 'E' characters for numeric attribute named '%s'" ,
                            sName);
      // exponent may have sign
      if (*p == '-' || *p == '+')
        p++;
      } // end of exponent
    else if(!isdigit (*p))
      ThrowErrorException ("Invalid character '%c' in numeric attribute named '%s'" ,
                          *p,
                          sName);

  
    }   // end of checking each character


  myAtoF (strValue, &dResult); 

  /*   // myAtoF does not return myAtoF

  if (dResult == c || dResult == -HUGE_VAL)
    ThrowErrorException ("Value '%s' out of range in numeric attribute named '%s'" ,
                        (LPCTSTR) strValue,
                        sName);

  */

  if (dResult > dMaximum)
    ThrowErrorException ("Value '%s' too large in numeric attribute named '%s'. "
                         "Range is %g to %g.",
                        (LPCTSTR) strValue,
                         sName,
                         dMinimum, 
                         dMaximum);

  if (dResult < dMinimum)
    ThrowErrorException ("Value '%s' too small in numeric attribute named '%s'. "
                         "Range is %g to %g.",
                        (LPCTSTR) strValue,
                         sName,
                         dMinimum, 
                         dMaximum);

  dValue = dResult;
  return true;


  } // end of Get_XML_double

bool Get_XML_short (CXMLelement & node,
                    const char * sName,
                    short & iValue,
                    const bool bUseDefault,
                    const long iMinimum,
                    const long iMaximum)
  {
  long lValue;

  if (!Get_XML_number (node, sName, lValue, bUseDefault, iMinimum, iMaximum))
    if (bUseDefault)
      return false;

  iValue = (short) lValue;
  return true;

  }  // end of Get_XML_short

bool Get_XML_int (CXMLelement & node,
                    const char * sName,
                    int & iValue,
                    const bool bUseDefault,
                    const long iMinimum,
                    const long iMaximum)
  {
  long lValue;

  if (!Get_XML_number (node, sName, lValue, bUseDefault, iMinimum, iMaximum))
    if (bUseDefault)
      return false;

  iValue = (int) lValue;
  return true;

  }  // end of Get_XML_int

bool Get_XML_long (CXMLelement & node,
                    const char * sName,
                    long & iValue,
                    const bool bUseDefault,
                    const long iMinimum,
                    const long iMaximum)
  {
  long lValue;

  if (!Get_XML_number (node, sName, lValue, bUseDefault, iMinimum, iMaximum))
    if (bUseDefault)
      return false;

  iValue = lValue;
  return true;

  }  // end of Get_XML_long

bool Get_XML_ushort (CXMLelement & node,
                    const char * sName,
                    unsigned short & iValue,
                    const bool bUseDefault,
                    const long iMinimum,
                    const long iMaximum)
  {
  long lValue;

  if (!Get_XML_number (node, sName, lValue, bUseDefault, iMinimum, iMaximum))
    if (bUseDefault)
      return false;

  iValue = (short) lValue;
  return true;

  }  // end of Get_XML_short

bool Get_XML_string (CXMLelement & node,
                        const char * sName,
                        CString & strValue,
                        const bool bUseDefault,
                        const bool bTrimSpaces)
  {
CAttribute * pAttribute = GetAttribute (node, sName);

  if (!pAttribute)
    if (bUseDefault)
      return false;
    else
      {
      strValue.Empty ();
      return false;
      }

  strValue = pAttribute->strValue;

  if (bTrimSpaces)
    {
    strValue.TrimLeft ();
    strValue.TrimRight ();
    }

  return true;

  }  // end of Get_XML_string

bool Get_XML_colour (CXMLelement & node,
                        const char * sName,
                        COLORREF & cValue,
                        const bool bUseDefault)

  {
CString strValue;

  if (!Get_XML_string (node, sName, strValue, true, true))
    if (bUseDefault)
      return false;
    else
      {
      cValue = 0;     // default is black
      return false;
      }

  if (strValue.IsEmpty ())
      ThrowErrorException ("No colour supplied for attribute named '%s'" ,
                          sName);

  if (SetColour (strValue, cValue)) 
      ThrowErrorException ("Invalid colour code \"%s\" for attribute named '%s'" ,
                          (LPCTSTR) strValue,
                          sName);
  return true;

  }  // end of Get_XML_colour

// tests a string to see if it is numeric
bool IsNumber (const CString & str, bool bAllowSign)
  {
const char * p = str;

  if (!*p)
    return false; // empty? can't be a number

  // skip leading minus sign
  if (bAllowSign && 
     (*p == '-' || *p == '+')
     )
    p++;

  for ( ; *p; p++)
    if (!isdigit (*p))
      return false;

  // see if out of range of a long
  __int64 iNum = _atoi64 (str);

  if (iNum < LONG_MIN || iNum > LONG_MAX)
    return false;  // too big

  return true;
  }   // end of IsNumber

bool Get_XML_date (CXMLelement & node,
                        const char * sName,
                        CTime & tValue,
                        const bool bUseDefault)

  {
CString strValue;

  if (!Get_XML_string (node, sName, strValue, true, true))
    if (bUseDefault)
      return false;
    else
      {
      tValue = 0;     // default is no time
      return false;
      }

  if (strValue.IsEmpty ())
      ThrowErrorException ("No time supplied for attribute named '%s'" ,
                          sName);

  // break up date into date portion / time portion
  CStringList strDateTime;
  StringToList (strValue, " ", strDateTime);

  if (strDateTime.GetCount () < 1 || strDateTime.GetCount () > 2)
    ThrowErrorException ("Date/time must consist of YYYY-MM-DD [ HH:MM:SS ]");

  CString strDate = strDateTime.RemoveHead ();
  CString strTime;
  if (!strDateTime.IsEmpty ())
     strTime = strDateTime.RemoveHead ();

  CString strYear, strMonth, strDay;

  CStringList strDateList;
  StringToList (strDate, "-", strDateList);

  if (strDateList.GetCount () != 3)
     ThrowErrorException ("Date must consist of YYYY-MM-DD");
   
  strYear = strDateList.RemoveHead ();
  strMonth = strDateList.RemoveHead ();
  strDay = strDateList.RemoveHead ();

  if (!IsNumber (strYear))
     ThrowErrorException ("Year is not numeric");
  if (!IsNumber (strMonth))
     ThrowErrorException ("Month is not numeric");
  if (!IsNumber (strDay))
     ThrowErrorException ("Day is not numeric");

  int iYear, iMonth, iDay;

  iYear = atoi (strYear);
  iMonth = atoi (strMonth);
  iDay = atoi (strDay);

  if (iYear <  1900 || iYear > 2100)
     ThrowErrorException ("Year must be in range 1900 to 2100");
  
  if (iMonth < 1 || iMonth > 12)
     ThrowErrorException ("Month must be in range 1 to 12");
  
  if (iDay < 1 || iDay > 31)
     ThrowErrorException ("Month must be in range 1 to 31");

  int iHour = 0, iMinute = 0, iSecond = 0;

  if (!strTime.IsEmpty ())
    {

    CString strHour, strMinute, strSecond;

    CStringList strTimeList;
    StringToList (strTime, ":", strTimeList);

    if (strTimeList.GetCount () > 3)
       ThrowErrorException ("Time must consist of HH, HH:MM, or HH:MM:SS");
   
    strHour = strTimeList.RemoveHead ();
    strMinute = "0";
    strSecond = "0";

    if (!strTimeList.IsEmpty ())
      {
      strMinute = strTimeList.RemoveHead ();
      if (!strTimeList.IsEmpty ())
        strSecond = strTimeList.RemoveHead ();
      }

    if (!IsNumber (strHour))
       ThrowErrorException ("Hour is not numeric");
    if (!IsNumber (strMinute))
       ThrowErrorException ("Minute is not numeric");
    if (!IsNumber (strSecond))
       ThrowErrorException ("Second is not numeric");

    iHour = atoi (strHour);
    iMinute = atoi (strMinute);
    iSecond = atoi (strSecond);

    if (iHour <  0 || iHour > 23)
       ThrowErrorException ("Hour must be in range 0 to 23");
  
    if (iMinute < 0 || iMinute > 59)
       ThrowErrorException ("Minute must be in range 0 to 59");
  
    if (iSecond < 0 || iSecond > 59)
       ThrowErrorException ("Minute must be in range 0 to 59");

    } // end of having a time

  tValue = CTime (iYear, iMonth, iDay, iHour, iMinute, iSecond, 0);

  return true;

  }  // end of Get_XML_date


// use this to find a node (triggers, aliases, general, etc.)

CXMLelement * FindNode (CXMLelement & parent,
                        const char * sName)                                 
  {

  for (POSITION pos = parent.ChildrenList.GetHeadPosition (); pos; )
    {
    CXMLelement * pElement = parent.ChildrenList.GetNext (pos);
    if (pElement->strName == sName)
      {
      pElement->bUsed = true; // we used it!
      // note line it was found on
      iLineLastItemFound = pElement->iLine;
      return pElement;
      }  // end of found tag
    } // end of searching for wanted tag

  return NULL;
  } // end of FindNode  

// use this to find a node with a string value (eg. <send>blah</send> )

// optional - trim argument, eg.
//   <send trim="1">
//      blah blah
//   </send>
//
// trim will trim carriage-return, linefeed, tab, space

bool FindNodeContents (CXMLelement & parent,
                       const char * sName,
                       CString & strValue,
                       const bool bUseDefault,    
                       bool bTrim)
  {
CXMLelement * pNode = FindNode (parent, sName);

  if (!pNode)
    if (bUseDefault)
      return false;
    else
      {
      strValue = "";
      return false;
      }

  // see if they want spaces trimmed
  Get_XML_boolean (*pNode, "trim", bTrim, true);

  strValue = pNode->strContent;

  if (bTrim)
    {
    strValue.TrimLeft ("\n\r\t ");
    strValue.TrimRight ("\n\r\t ");
    }

  return true;

  } // end of FindNode  
