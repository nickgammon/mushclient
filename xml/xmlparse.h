#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <float.h>
#include <math.h>

#define MAX_XML_DOCUMENT_SIZE (5000 * 1024)   // max XML file length - 5,000 Kb
#define MAX_XML_NAME_LENGTH 255        // max name of element (eg. <trigger> )

class CXMLelement;

class CAttribute : public CObject
  {

  public:

  CString strName;          // element name (eg. colour)
  CString strValue;         // value (eg. red )
  bool    bUsed;            // true if used in document
  UINT    iLine;            // line it appeared on in XML document

  CAttribute () 
    {
    bUsed = false;
    };    // constructor


  };

typedef CTypedPtrMap <CMapStringToPtr, CString, CAttribute*> CAttributeMap;

typedef CTypedPtrList <CPtrList, CXMLelement*> CElementList;

class CXMLelement : public CObject
  {

  public:

  CString strName;                  // element name (eg. <triggers>)
  CAttributeMap   AttributeMap;     // list of attributes to this element (colour="red")
                                    //  nb - attributes must be unique, so we use a map
  CElementList    ChildrenList;     // children (eg. <trigger> )
  CString         strContent;       // content (eg. <match>blah blah</match> )
                                    //   in this case content would be "blah blah"

  bool            bEmpty;           // if empty, eg. <br />  - no closing tag
  bool            bUsed;            // true if used in document
  UINT            iLine;            // line it appeared on in XML document

  CXMLelement () 
    {
    bEmpty = false;
    bUsed = false;
    iLine = 1;
    AttributeMap.InitHashTable (127);
    };    // constructor

  // destructor deletes attributes and children
  ~CXMLelement () 
    { 
    DELETE_LIST (ChildrenList); 
    DELETE_MAP  (AttributeMap, CAttribute);
    };  // destructor

  };

class CXMLparser : public CObject
  {

  const char * m_xmlBuff; // pointer into buffer for XML parsing
  CString m_strxmlBuffer;   // buffer for parsing
  UINT m_xmlLength;  // size of XML buffer
  UINT m_xmlPos;     // where we are in the buffer

  void SkipComments (const bool bAndSpaces);
  void SkipSpaces (void);
  void ProcessNode (CXMLelement & parent);
  void ProcessAttributes (CXMLelement & node);
  void ProcessDeclaration (const CString strDeclaration);
  void ProcessDoctype (void);
  void ProcessDTD (void);
  void ProcessEntity (void);
  void AssembleContent (CXMLelement & node, 
                        const UINT iFirstLine, 
                        const UINT iNodeStart);

  CString ReplaceEntities (const CString strSource);
  CString GetName (const char * sType);
  CString GetValue (const char * sType, const char * sName);

  public:

  CMapStringToString m_CustomEntityMap;    // custom XML entities

  CString Get_XML_Entity (CString & strName);

  CXMLparser ()
    {
    m_xmlBuff = NULL; // no buffer
    m_xmlLine = 1;  // first line is line 1
    m_xmlPos = 0;   // start at start of buffer
    m_xmlLength = 0;

    }; // end of constructor

  ~CXMLparser ()
    {
    m_strxmlBuffer.Empty ();  
    }; // end of destructor

  CXMLelement   m_xmlRoot;          // root element
  UINT          m_xmlLine;          // which line we are up to
  CString       m_strDocumentName;  // root document name  (from <!DOCTYPE name .. ?> )

  // builds an XML structure from the supplied (and open) file

  void BuildStructure (CFile * file);

  };

// gets an attribute from an element - marks it as used

CAttribute * GetAttribute (CXMLelement & node,
                           const char * sName);

bool Get_XML_boolean (CXMLelement & node,
                          const char * sName,
                          bool & bValue,
                          const bool bUseDefault);

bool Get_XML_boolean_ushort (CXMLelement & node,
                          const char * sName,
                          unsigned short & bValue,
                          const bool bUseDefault);
                   
bool Get_XML_boolean_BOOL (CXMLelement & node,
                          const char * sName,
                          BOOL & bValue,
                          const bool bUseDefault);

bool Get_XML_boolean_ushort_mask (CXMLelement & node,
                          const char * sName,
                          unsigned short & bValue,
                          const unsigned short iMask,
                          const bool bUseDefault);

bool Get_XML_boolean_int_mask (CXMLelement & node,
                          const char * sName,
                          int & bValue,
                          const int iMask,
                          const bool bUseDefault);

bool Get_XML_number (CXMLelement & node,
                        const char * sName,
                        long & iValue,
                        const bool bUseDefault,
                        long iMinimum = LONG_MIN,
                        long iMaximum = LONG_MAX);

bool Get_XML_short (CXMLelement & node,
                        const char * sName,
                        short & iValue,
                        const bool bUseDefault,
                        const long iMinimum = SHRT_MIN,
                        const long iMaximum = SHRT_MAX);

bool Get_XML_int  (CXMLelement & node,
                        const char * sName,
                        int & iValue,
                        const bool bUseDefault,
                        const long iMinimum = INT_MIN,
                        const long iMaximum = INT_MAX);

bool Get_XML_long  (CXMLelement & node,
                        const char * sName,
                        long & iValue,
                        const bool bUseDefault,
                        const long iMinimum = LONG_MIN,
                        const long iMaximum = LONG_MAX);

bool Get_XML_double  (CXMLelement & node,
                        const char * sName,
                        double & dValue,
                        const bool bUseDefault,
                        const double dMinimum = -DBL_MAX,   // not DBL_MIN
                        const double dMaximum = DBL_MAX);

bool Get_XML_ushort (CXMLelement & node,
                        const char * sName,
                        unsigned short & iValue,
                        const bool bUseDefault,
                        const long iMinimum = 0,
                        const long iMaximum = USHRT_MAX);

bool Get_XML_string (CXMLelement & node,
                        const char * sName,
                        CString & strValue,
                        const bool bUseDefault,
                        const bool bTrimSpaces = false);

bool Get_XML_colour (CXMLelement & node,
                        const char * sName,
                        COLORREF & cValue,
                        const bool bUseDefault);

bool Get_XML_date (CXMLelement & node,
                        const char * sName,
                        CTime & tValue,
                        const bool bUseDefault);


CXMLelement * FindNode (CXMLelement & parent,
                        const char * sName);


bool FindNodeContents (CXMLelement & parent,
                       const char * sName,
                       CString & strValue,
                       const bool bUseDefault,
                       bool bTrim = false);

