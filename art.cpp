/*

  ASCII-art generator.

  Based on figlet.

  FIGlet Copyright 1991, 1993, 1994 Glenn Chappell and Ian Chai
  FIGlet Copyright 1996, 1997 John Cowan
  Portions written by Paul Burton
  Internet: <ianchai@usa.net>
  FIGlet, along with the various FIGlet fonts and documentation, is
    copyrighted under the provisions of the Artistic License (as listed
    in the file "artistic.license" which is included in this package.

  */

#include "stdafx.h"
#include "MUSHclient.h"
#include "TextDocument.h"
#include "TextView.h"
#include "doc.h"
#include "dialogs\AsciiArtDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define MYSTRLEN(x) ((int)strlen(x)) /* Eliminate ANSI problem */
#define FONTFILESUFFIX ".flf"
#define FONTFILEMAGICNUMBER "flf2"
#define FSUFFIXLEN MYSTRLEN(FONTFILESUFFIX)
#define CONTROLFILESUFFIX ".flc"
#define CONTROLFILEMAGICNUMBER "flc2"   /* no longer used in 2.2 */
#define CSUFFIXLEN MYSTRLEN(CONTROLFILESUFFIX)
#define DEFAULTCOLUMNS 500

#ifndef DEFAULTFONTDIR
#define DEFAULTFONTDIR "fonts"
#endif
#ifndef DEFAULTFONTFILE
#define DEFAULTFONTFILE "standard.flf"
#endif

/****************************************************************************

  Globals dealing with chars that are read

****************************************************************************/

typedef long inchr; /* "char" read from stdin */

static inchr *inchrline = NULL;  /* Alloc'd inchr inchrline[inchrlinelenlimit+1]; */
                   /* Note: not null-terminated. */
static int inchrlinelen,inchrlinelenlimit;
static inchr deutsch[7] = {196, 214, 220, 228, 246, 252, 223};
  /* Latin-1 codes for German letters, respectively:
     LATIN CAPITAL LETTER A WITH DIAERESIS = A-umlaut
     LATIN CAPITAL LETTER O WITH DIAERESIS = O-umlaut
     LATIN CAPITAL LETTER U WITH DIAERESIS = U-umlaut
     LATIN SMALL LETTER A WITH DIAERESIS = a-umlaut
     LATIN SMALL LETTER O WITH DIAERESIS = o-umlaut
     LATIN SMALL LETTER U WITH DIAERESIS = u-umlaut
     LATIN SMALL LETTER SHARP S = ess-zed
  */

/****************************************************************************

  Globals affected by command line options

****************************************************************************/

static int deutschflag,justification,paragraphflag,right2left,multibyte;
static int cmdinput;

#define SM_SMUSH 128
#define SM_KERN 64
#define SM_EQUAL 1
#define SM_LOWLINE 2
#define SM_HIERARCHY 4
#define SM_PAIR 8
#define SM_BIGX 16
#define SM_HARDBLANK 32

static int smushmode;

#define SMO_NO 0     /* no command-line smushmode */
#define SMO_YES 1    /* use command-line smushmode, ignore font smushmode */
#define SMO_FORCE 2  /* logically OR command-line and font smushmodes */

static int smushoverride;

static int outputwidth;
static int outlinelenlimit;
static char *fontdirname,*fontname;


/****************************************************************************

  Globals dealing with chars that are written

****************************************************************************/

typedef struct fc {
  inchr ord;
  char **thechar;  /* Alloc'd char thechar[charheight][]; */
  struct fc *next;
  } fcharnode;

static fcharnode *fcharlist = NULL;
static char **currchar;
static int currcharwidth;
static int previouscharwidth;
static char **outputline = NULL;    /* Alloc'd char outputline[charheight][outlinelenlimit+1]; */
static int outlinelen;

/****************************************************************************

  Globals read from font file

****************************************************************************/

static char hardblank;
static int charheight;

/****************************************************************************

  myalloc

  Calls malloc.  If malloc returns error, prints error message and
  quits.

****************************************************************************/

/*
char *myalloc(size_t size)
{
  char *ptr;

  ptr = new char [size];

  if (ptr == NULL)
    ThrowErrorException ("Out of memory");

  return ptr;
}

*/

#define myalloc(arg) new char [arg]
#define myfree(arg) delete [] arg


/****************************************************************************

  getparams

  Handles all command-line parameters.  Puts all parameters within
  bounds.

****************************************************************************/

static void getparams()
{
  extern char *optarg;
  extern int optind;
  int firstfont,infoprint;

  fontdirname = DEFAULTFONTDIR;
  firstfont = 1;
  deutschflag = 0;
  justification = -1;
  right2left = -1;
  paragraphflag = 0;
  infoprint = -1;
  cmdinput = 0;
  outputwidth = DEFAULTCOLUMNS;
  
  outlinelenlimit = outputwidth-1;
}

/****************************************************************************

  readmagic

  Reads a four-character magic string from a stream.

****************************************************************************/
static void readmagic(CStdioFile & file, char * magic)
{

  file.Read (magic, 4);
  magic[4] = 0;
  }

/****************************************************************************

  skiptoeol

  Skips to the end of a line, given a stream. 

****************************************************************************/

static void skiptoeol(CStdioFile & fp)
{
  char dummy [2];

  while (fp.Read (dummy, 1)) 
    if (dummy [0] == '\n') return;
}


/****************************************************************************

  readfontchar

  Reads a font character from the font file, and places it in a
  newly-allocated entry in the list.

****************************************************************************/

static void readfontchar(CStdioFile & file, inchr theord, char * line, int maxlen)
{
  int row,k;
  char endchar;
  fcharnode *fclsave;

  fclsave = fcharlist;
  fcharlist = (fcharnode*)myalloc(sizeof(fcharnode));
  fcharlist->ord = theord;
  fcharlist->thechar = (char**)myalloc(sizeof(char*)*charheight);
  fcharlist->next = fclsave;
  for (row=0;row<charheight;row++) {
    if (file.ReadString(line,maxlen+1)==NULL) {
      line[0] = '\0';
      }
    k = MYSTRLEN(line)-1;
    while (k>=0 && isspace(line[k])) {
      k--;
      }
    if (k>=0) {
      endchar = line[k];
      while (k>=0 ? line[k]==endchar : 0) {
        k--;
        }
      }
    line[k+1] = '\0';
    fcharlist->thechar[row] = (char*)myalloc(sizeof(char)*(k+2));
    strcpy(fcharlist->thechar[row],line);
    }
}


/****************************************************************************

  readfont

  Allocates memory, initializes variables, and reads in the font.

****************************************************************************/

static void readfont(CString strName)
{
#define MAXFIRSTLINELEN 1000
  int i,row,numsread;
  inchr theord;
  int maxlen,cmtlines,ffright2left;
  int smush,smush2;
  char *fileline,magicnum[5];


  CStdioFile fontfile (strName, 
                        CFile::modeRead|CFile::shareDenyNone|CFile::typeText);

  readmagic(fontfile,magicnum);
  fileline = (char*)myalloc(sizeof(char)*(MAXFIRSTLINELEN+1));
  if (fontfile.ReadString(fileline,MAXFIRSTLINELEN+1)==NULL) {
    fileline[0] = '\0';
    }
  if (MYSTRLEN(fileline)>0 ? fileline[MYSTRLEN(fileline)-1]!='\n' : 0) {
    skiptoeol(fontfile);
    }
  numsread = sscanf(fileline,"%*c%c %d %*d %d %d %d %d %d",
    &hardblank,&charheight,&maxlen,&smush,&cmtlines,
    &ffright2left,&smush2);
  myfree(fileline);
  if (strcmp(magicnum,FONTFILEMAGICNUMBER) || numsread<5)
    ThrowErrorException ("Not a FIGlet 2 font file");
  for (i=1;i<=cmtlines;i++) {
    skiptoeol(fontfile);
    }

  if (numsread<6) {
    ffright2left = 0;
    }

  if (numsread<7) { /* if no smush2, decode smush into smush2 */
    if (smush == 0) smush2 = SM_KERN;
    else if (smush < 0) smush2 = 0;
    else smush2 = (smush & 31) | SM_SMUSH;
    }

  if (charheight<1) {
    charheight = 1;
    }

  if (maxlen<1) {
    maxlen = 1;
    }

  maxlen += 100; /* Give ourselves some extra room */

  if (smushoverride == SMO_NO)
     smushmode = smush2;
  else if (smushoverride == SMO_FORCE)
     smushmode |= smush2;

  if (right2left<0) {
    right2left = ffright2left;
    }

  if (justification<0) {
    justification = 2*right2left;
    }

  fileline = (char*)myalloc(sizeof(char)*(maxlen+1));
  /* Allocate "missing" character */
  fcharlist = (fcharnode*)myalloc(sizeof(fcharnode));
  fcharlist->ord = 0;
  fcharlist->thechar = (char**)myalloc(sizeof(char*)*charheight);
  fcharlist->next = NULL;
  for (row=0;row<charheight;row++) {
    fcharlist->thechar[row] = (char*)myalloc(sizeof(char));
    fcharlist->thechar[row][0] = '\0';
    }
  for (theord=' ';theord<='~';theord++) {
    readfontchar(fontfile,theord,fileline,maxlen);
    }
  for (theord=0;theord<=6;theord++) {
    readfontchar(fontfile,deutsch[theord],fileline,maxlen);
    }
  while (fontfile.ReadString(fileline,maxlen+1)==NULL?0:
    sscanf(fileline,"%li",&theord)==1) {
    readfontchar(fontfile,theord,fileline,maxlen);
    }
  myfree(fileline);
}

/****************************************************************************

  clearline

  Clears both the input (inchrline) and output (outputline) storage.

****************************************************************************/

static void clearline()
{
  int i;

  for (i=0;i<charheight;i++) {
    outputline[i][0] = '\0';
    }
  outlinelen = 0;
  inchrlinelen = 0;
}

/****************************************************************************

  linealloc

  Allocates & clears outputline, inchrline. Sets inchrlinelenlimit.
  Called near beginning of main().

****************************************************************************/

static void linealloc()
{
  int row; 

  outputline = (char**)myalloc(sizeof(char*)*charheight);
  for (row=0;row<charheight;row++) {
    outputline[row] = (char*)myalloc(sizeof(char)*(outlinelenlimit+1));
    }
  inchrlinelenlimit = outputwidth*4+100;
  inchrline = (inchr*)myalloc(sizeof(inchr)*(inchrlinelenlimit+1));
  clearline();
}

/****************************************************************************

  linefree

  Frees outputline, inchrline. 

****************************************************************************/

static void linefree()
{
  int row; 

  if (outputline)
    {

    for (row=0;row<charheight;row++) {
      if (outputline[row])
        myfree (outputline[row]);
      outputline[row] = NULL;
      }
    
    myfree (outputline);
    outputline = NULL;
    }

  if (inchrline)
    myfree (inchrline);
  inchrline = NULL;

}

static void fontfree()
  {
  int row; 
  fcharnode *charptr, *nextptr;

  for (charptr=fcharlist; charptr; charptr=nextptr)
    {
    nextptr = charptr->next;    // next node
    if (charptr->thechar)
      {
      for (row=0;row<charheight;row++) {
        if (charptr->thechar[row])
          myfree (charptr->thechar[row]);
        charptr->thechar[row] = NULL;
        }

      myfree (charptr->thechar);
      charptr->thechar = NULL;
      }

    myfree (charptr);
    charptr = NULL;
    }   // end of traversing list
  fcharlist = NULL; // list is empty now
  }

/****************************************************************************

  getletter

  Sets currchar to point to the font entry for the given character.
  Sets currcharwidth to the width of this character.

****************************************************************************/

static void getletter(inchr c)
{
  fcharnode *charptr;

  for (charptr=fcharlist;charptr==NULL?0:charptr->ord!=c;
    charptr=charptr->next) ;
  if (charptr!=NULL) {
    currchar = charptr->thechar;
    }
  else {
    for (charptr=fcharlist;charptr==NULL?0:charptr->ord!=0;
      charptr=charptr->next) ;
    currchar = charptr->thechar;
    }
  previouscharwidth = currcharwidth;
  currcharwidth = MYSTRLEN(currchar[0]);
}


/****************************************************************************

  smushem

  Given 2 characters, attempts to smush them into 1, according to
  smushmode.  Returns smushed character or '\0' if no smushing can be
  done.

  smushmode values are sum of following (all values smush blanks):
    1: Smush equal chars (not hardblanks)
    2: Smush '_' with any char in hierarchy below
    4: hierarchy: "|", "/\", "[]", "{}", "()", "<>"
       Each class in hier. can be replaced by later class.
    8: [ + ] -> |, { + } -> |, ( + ) -> |
   16: / + \ -> X, > + < -> X (only in that order)
   32: hardblank + hardblank -> hardblank

****************************************************************************/

static char smushem(char lch,char rch)
{
  if (lch==' ') return rch;
  if (rch==' ') return lch;

  if (previouscharwidth<2 || currcharwidth<2) return '\0';
    /* Disallows overlapping if the previous character */
    /* or the current character has a width of 1 or zero. */

  if ((smushmode & SM_SMUSH) == 0) return '\0';  /* kerning */

  if ((smushmode & 63) == 0) {
    /* This is smushing by universal overlapping. */
    if (lch==' ') return rch;
    if (rch==' ') return lch;
    if (lch==hardblank) return rch;
    if (rch==hardblank) return lch;
      /* Above four lines ensure overlapping preference to */
      /* visible characters. */
    if (right2left==1) return lch;
      /* Above line ensures that the dominant (foreground) */
      /* fig-character for overlapping is the latter in the */
      /* user's text, not necessarily the rightmost character. */
    return rch;
      /* Occurs in the absence of above exceptions. */
    }
  
  if (smushmode & SM_HARDBLANK) {
    if (lch==hardblank && rch==hardblank) return lch;
    }

  if (lch==hardblank || rch==hardblank) return '\0';

  if (smushmode & SM_EQUAL) {
    if (lch==rch) return lch;
    }

  if (smushmode & SM_LOWLINE) {
    if (lch=='_' && strchr("|/\\[]{}()<>",rch)) return rch;
    if (rch=='_' && strchr("|/\\[]{}()<>",lch)) return lch;
    }

  if (smushmode & SM_HIERARCHY) {
    if (lch=='|' && strchr("/\\[]{}()<>",rch)) return rch;
    if (rch=='|' && strchr("/\\[]{}()<>",lch)) return lch;
    if (strchr("/\\",lch) && strchr("[]{}()<>",rch)) return rch;
    if (strchr("/\\",rch) && strchr("[]{}()<>",lch)) return lch;
    if (strchr("[]",lch) && strchr("{}()<>",rch)) return rch;
    if (strchr("[]",rch) && strchr("{}()<>",lch)) return lch;
    if (strchr("{}",lch) && strchr("()<>",rch)) return rch;
    if (strchr("{}",rch) && strchr("()<>",lch)) return lch;
    if (strchr("()",lch) && strchr("<>",rch)) return rch;
    if (strchr("()",rch) && strchr("<>",lch)) return lch;
    }

  if (smushmode & SM_PAIR) {
    if (lch=='[' && rch==']') return '|';
    if (rch=='[' && lch==']') return '|';
    if (lch=='{' && rch=='}') return '|';
    if (rch=='{' && lch=='}') return '|';
    if (lch=='(' && rch==')') return '|';
    if (rch=='(' && lch==')') return '|';
    }

  if (smushmode & SM_BIGX) {
    if (lch=='/' && rch=='\\') return '|';
    if (rch=='/' && lch=='\\') return 'Y';
    if (lch=='>' && rch=='<') return 'X';
      /* Don't want the reverse of above to give 'X'. */
    }

  return '\0';
}


/****************************************************************************

  smushamt

  Returns the maximum amount that the current character can be smushed
  into the current line.

****************************************************************************/

static int smushamt()
{
  int maxsmush,amt;
  int row,linebd,charbd;
  char ch1,ch2;

  if ((smushmode & (SM_SMUSH | SM_KERN)) == 0) {
    return 0;
    }
  maxsmush = currcharwidth;
  for (row=0;row<charheight;row++) {
    if (right2left) {
      for (charbd=MYSTRLEN(currchar[row]);
        ch1=currchar[row][charbd],(charbd>0&&(!ch1||ch1==' '));charbd--) ;
      for (linebd=0;ch2=outputline[row][linebd],ch2==' ';linebd++) ;
      amt = linebd+currcharwidth-1-charbd;
      }
    else {
      for (linebd=MYSTRLEN(outputline[row]);
        ch1 = outputline[row][linebd],(linebd>0&&(!ch1||ch1==' '));linebd--) ;
      for (charbd=0;ch2=currchar[row][charbd],ch2==' ';charbd++) ;
      amt = charbd+outlinelen-1-linebd;
      }
    if (!ch1||ch1==' ') {
      amt++;
      }
    else if (ch2) {
      if (smushem(ch1,ch2)!='\0') {
        amt++;
        }
      }
    if (amt<maxsmush) {
      maxsmush = amt;
      }
    }
  return maxsmush;
}


/****************************************************************************

  addchar

  Attempts to add the given character onto the end of the current line.
  Returns 1 if this can be done, 0 otherwise.

****************************************************************************/

static int addchar(inchr c)
{
  int smushamount,row,k;
  char *templine;

  getletter(c);
  smushamount = smushamt();
  if (outlinelen+currcharwidth-smushamount>outlinelenlimit
      ||inchrlinelen+1>inchrlinelenlimit) {
    return 0;
    }

  templine = (char*)myalloc(sizeof(char)*(outlinelenlimit+1));
  for (row=0;row<charheight;row++) {
    if (right2left) {
      strcpy(templine,currchar[row]);
      for (k=0;k<smushamount;k++) {
        templine[currcharwidth-smushamount+k] =
          smushem(templine[currcharwidth-smushamount+k],outputline[row][k]);
        }
      strcat(templine,outputline[row]+smushamount);
      strcpy(outputline[row],templine);
      }
    else {
      for (k=0;k<smushamount;k++) {
        outputline[row][outlinelen-smushamount+k] =
          smushem(outputline[row][outlinelen-smushamount+k],currchar[row][k]);
        }
      strcat(outputline[row],currchar[row]+smushamount);
      }
    }
  myfree(templine);
  outlinelen = MYSTRLEN(outputline[0]);
  inchrline[inchrlinelen++] = c;
  return 1;
}


void CTextView::OnEditAsciiart() 
{

CAsciiArtDlg dlg;

  dlg.m_strText = App.m_strAsciiArtText;
  dlg.m_iLayout = App.m_iAsciiArtLayout;
  dlg.m_strFont = App.m_strAsciiArtFont;

  if (dlg.DoModal () != IDOK)
    return;

  // remember text but don't save in registry
  App.m_strAsciiArtText = dlg.m_strText;

  // remember layout in registry
  if (dlg.m_iLayout != (int) App.m_iAsciiArtLayout)
    App.db_write_int ("prefs", "AsciiArtLayout", dlg.m_iLayout);	
  App.m_iAsciiArtLayout = dlg.m_iLayout;

  // remember font in registry
  if (dlg.m_strFont != App.m_strAsciiArtFont)
    App.db_write_string ("prefs", "AsciiArtFont", dlg.m_strFont);	
  App.m_strAsciiArtFont = dlg.m_strFont;

CString str = dlg.m_strText;

  switch (dlg.m_iLayout)
    {
    case 1:   // full smush
        smushmode = SM_SMUSH;
      	smushoverride = SMO_FORCE;
        break;

    case 2:   // kern
        smushmode = SM_KERN;
        smushoverride = SMO_YES;
        break;

    case 3:   // full width
        smushmode = 0;
      	smushoverride = SMO_YES;
        break;

    case 4:   // overlap
        smushmode = SM_SMUSH;
      	smushoverride = SMO_YES;
        break;

    default:  // default is normal smush
        smushoverride = SMO_NO;
        break;

    } // end of switch

inchr c;
int i;

	try
	  {
    getparams ();
    readfont (dlg.m_strFont);	
    linealloc();

    for (i = 0; i < str.GetLength (); i++) {

      c = str [i];

      if (isascii(c)&&isspace(c)) {
        c = (c=='\t'||c==' ') ? ' ' : '\n';
        }

      if ((c>'\0' && c<' ' && c!='\n') || c==127) continue;


      addchar (c);

      } // end of processing each character


    for (i=0;i<charheight;i++) {
      CString strLine = Replace (outputline[i], hardblank, " ");
      GetEditCtrl ().ReplaceSel (CFormat ("%s%s", (LPCTSTR) strLine, ENDLINE), true);
      }

    linefree ();
    fontfree ();
    
    }
	catch (CException* e)
	  {
		e->ReportError();
		e->Delete();
    linefree ();    // free memory used
    fontfree ();
	  }
}
