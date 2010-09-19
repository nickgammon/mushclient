#include "..\lua.h"
#include "..\lauxlib.h"

#include "..\zlib\zlib.h"
#include "shs.h"
#include "sha256.h"
#include "md5.h"
#include <io.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#pragma warning( disable : 4057)  // 'unsigned char *' differs in indirection to slightly different base types from 'char *'

typedef unsigned char UC;

// compression for use in MUSHclient Lua

static int optboolean (lua_State *L, const int narg, const int def) 
  {
  // that argument not present, take default
  if (lua_gettop (L) < narg)
    return def;

  // nil will default to the default
  if (lua_isnil (L, narg))
    return def;

  if (lua_isboolean (L, narg))
    return lua_toboolean (L, narg);

  return luaL_checknumber (L, narg) != 0;
}

// compress a string, returns compressed string
// optional argument - compression type

//  No compression         0  
//  Best speed             1  
//  Best compression       9  
//  Default compression   -1

static int mycompress (lua_State *L)
  {
  z_stream c_stream;

  // get text to compress
  size_t textLength;
  const char * text = luaL_checklstring (L, 1, &textLength);

  // get method, defaults to default method
  int method = (int) luaL_optnumber (L, 2, Z_DEFAULT_COMPRESSION);

  // output length must be input length + 0.1% + 12 bytes
  size_t comprLen = textLength + ((textLength + 999) / 1000) + 12
        + 50; // plus 50 for luck ;)

  char * compr = malloc (comprLen);

  if (!compr)
    luaL_error (L, "not enough memory for compression");

  c_stream.zalloc = Z_NULL; 
  c_stream.zfree = Z_NULL; 
  c_stream.opaque = Z_NULL; 

  c_stream.next_in  = (char *) text;
  c_stream.avail_in = textLength;
  c_stream.total_in = 0;

  if (deflateInit (&c_stream, method) != Z_OK)
      luaL_error (L, "could not initialize compression engine");

  c_stream.next_out = compr;
  c_stream.avail_out = (uInt)comprLen;
  c_stream.total_out = 0;

  // we can do it in one pass, hopefully ;)
  if (deflate (&c_stream, Z_FINISH) != Z_STREAM_END)
    luaL_error (L, "error on compression");

  if (deflateEnd (&c_stream) != Z_OK)
    luaL_error (L, "error on compression wrapup");

  lua_pushlstring (L, compr, c_stream.total_out);

  free (compr); // done with our compressed buffer
  return 1;  // number of result fields
  } // end of mycompress


// decompress a string, returns uncompressed string
static int mydecompress (lua_State *L)
  {
  z_stream d_stream;
  int err;

  // we'll push data into a Lua string buffer
  luaL_Buffer buffer;
  // however we'll use buf to do each chunk into
  char buf [1024];

  // get text to decompress
  size_t textLength;
  const char * text = luaL_checklstring (L, 1, &textLength);


  d_stream.zalloc = Z_NULL; 
  d_stream.zfree = Z_NULL; 
  d_stream.opaque = Z_NULL; 

  d_stream.next_in  = (char *) text;
  d_stream.avail_in = textLength;
  d_stream.total_in = 0;

  if (inflateInit (&d_stream) != Z_OK)
      luaL_error (L, "could not initialize decompression engine");

  luaL_buffinit(L, &buffer);

  d_stream.total_out = 0;

  do
    {
    d_stream.next_out = buf;
    d_stream.avail_out = sizeof (buf);

    err = inflate (&d_stream, Z_NO_FLUSH);
    if (err != Z_OK && err != Z_STREAM_END)
      luaL_error (L, "error on decompression");

    luaL_addlstring(&buffer, buf, d_stream.next_out - buf);
    
    } while (err != Z_STREAM_END);

  if (inflateEnd (&d_stream) != Z_OK)
    luaL_error (L, "error on decompression wrapup");

  luaL_pushresult(&buffer);

  return 1;  // number of result fields
  } // end of mydecompress

static int myhash (lua_State *L)
  {
  // get text to hash
  size_t textLength;
  const char * text = luaL_checklstring (L, 1, &textLength);
  char buf [50];

  SHS_INFO shsInfo;
  shsInit   (&shsInfo);
  shsUpdate (&shsInfo, (UC *) text, textLength);
  shsFinal  (&shsInfo);

  sprintf (buf, "%08x%08x%08x%08x%08x", 
                shsInfo.digest [0],
                shsInfo.digest [1],
                shsInfo.digest [2],
                shsInfo.digest [3],
                shsInfo.digest [4]
                );

  lua_pushstring (L, buf);

  return 1;  // number of result fields
  } // end of myhash

// SHA 256-bit hashing algorithm
// see: http://www.cr0.net:8040/code/crypto/sha256/

static int utils_sha256 (lua_State *L)
  {
  unsigned char digest [32];
  // get text to hash
  size_t textLength;
  const char * text = luaL_checklstring (L, 1, &textLength);

  sha256_context ctx;
  sha256_starts (&ctx);
  sha256_update (&ctx, (UC *) text, textLength);
  sha256_finish (&ctx, digest);

  lua_pushlstring (L, digest, sizeof digest);

  return 1;  // number of result fields
  } // end of utils_sha256

// MD5 128-bit hashing algorithm
// see: http://www.cr0.net:8040/code/crypto/md5/

static int utils_md5 (lua_State *L)
  {
  unsigned char digest [16];
  // get text to hash
  size_t textLength;
  const char * text = luaL_checklstring (L, 1, &textLength);

  md5_context ctx;
  md5_starts (&ctx);
  md5_update (&ctx, (UC *) text, textLength);
  md5_finish (&ctx, digest);

  lua_pushlstring (L, digest, sizeof digest);

  return 1;  // number of result fields
  } // end of utils_md5

static int utils_tohex (lua_State *L)
  {
  unsigned char buf [3];
  // we'll push data into a Lua string buffer
  luaL_Buffer buffer;

  // get text to convert
  size_t textLength;
  size_t i;
  const unsigned char * text = luaL_checklstring (L, 1, &textLength);

  // initialize buffer
  luaL_buffinit(L, &buffer);

  for (i = 0; i < textLength; i++)
    {
    sprintf (buf, "%02X", text [i]);
    luaL_addlstring(&buffer, buf, 2);
    }

  luaL_pushresult(&buffer);
  return 1;  // number of result fields
  } // end of utils_tohex


static int utils_fromhex (lua_State *L)
  {
  // we'll push data into a Lua string buffer
  luaL_Buffer buffer;

  // get text to convert
  const unsigned char * text = luaL_checkstring (L, 1);
  const unsigned char * p;

  // initialize buffer
  luaL_buffinit(L, &buffer);

  for (p = text; *p; )
    {
    size_t i;
    unsigned char c = 0, 
                  t;

    for (i = 0; *p && i < 2; p++)
      {
      // skip spaces
      if (isspace (*p))
        continue;

      if (!isxdigit (*p))
        luaL_error (L, "Not a hex digit ('%c') at position %d",
                    *p, (int) ((p - text) + 1));

      t = toupper (*p);
      if (t >= 'A')
        t -= 7;
    	c = (c << 4) + t - '0';
      i++;
      } // end of building up hex literal

    if (i)
      luaL_addlstring(&buffer, &c, 1);
    }

  luaL_pushresult(&buffer);
  return 1;  // number of result fields
  } // end of utils_fromhex

// to wrap at column 76 we actually add a linefeed after doing 57 bytes
#define WRAP_COLUMN 76
#define WRAP_POINT ((WRAP_COLUMN / 4) * 3)

/**************************************************************************/
static UC base64code[64]=
{
	'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
	'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
	'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
	'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
};


static int myencodebase64 (lua_State *L)
  {
  // get text to hash
  size_t textLength;
  const UC * text = luaL_checklstring (L, 1, &textLength);
  int bMultiLine = optboolean (L, 2, 0);

	size_t i;
	size_t j=0;
	int bytes;
  int bufsize;
  char * result = NULL;

  // result will be 4/3 size of original  
  // plus padding for final couple of bytes

  bufsize = ((textLength / 3) * 4) + 4;

  if (bMultiLine)
     bufsize += (((textLength / WRAP_POINT) + 1) * 2);  // (allow 2 bytes for cr/lfs)

  result = malloc (bufsize);

  if (!result)
    luaL_error (L, "not enough memory for encoding");

	// go thru converting each 3 bytes into 4 base64 bytes
  if (textLength >= 3)
    {
    for (i=0; i < (textLength - 2); i+=3)
      {
		  bytes = (text [i] << 16)
		 	      + (text [i+1] << 8)
			      + (text [i+2]);
		  result[j++] = base64code[(bytes >> 18)  & 0x3F];
		  result[j++] = base64code[(bytes >> 12)  & 0x3F];
		  result[j++] = base64code[(bytes >> 6)   & 0x3F];
		  result[j++] = base64code[(bytes)        & 0x3F];

    // Add linefeeds every WRAP_POINT characters

		  if(bMultiLine && ((i % WRAP_POINT) == (WRAP_POINT - 3)))
        {                 
			  result[j++]='\r';
			  result[j++]='\n';
        }
	    }
    }
  else
    i = 0;

	// pad the remaining characters
	switch(textLength - i){
		case 0: // exact fit
			break;
		case 1: // need one more character
			bytes= text [i]<<16;
			result[j++] = base64code[(bytes>>18) & 0x3F];
			result[j++] = base64code[(bytes>>12) & 0x3F];
			result[j++] = '=';
			result[j++] = '=';
			break;
		case 2: // need two more characters 
			bytes= (text [i]<<16) + (text [i+1]<<8);
			result[j++] = base64code[(bytes>>18) & 0x3F];
			result[j++] = base64code[(bytes>>12) & 0x3F];
			result[j++] = base64code[(bytes>>6) & 0x3F];
			result[j++] = '=';
			break;

	}

  lua_pushlstring (L, result, j);

  free (result); // done with our buffer
  return 1;  // number of result fields

  } // end of myencodebase64

static UC b64unbase [256];

/*-------------------------------------------------------------------------*\
* Fill base64 decode map. 
\*-------------------------------------------------------------------------*/
static void b64setup (void) 
{
    int i;
    for (i = 0; i < 255; i++) b64unbase[i] = 255;
    for (i = 0; i < 64; i++) b64unbase[base64code[i]] = i;
    b64unbase['='] = 0;
}

/*-------------------------------------------------------------------------*\
* Acumulates bytes in input buffer until 4 bytes are available. 
* Translate the 4 bytes from Base64 form and append to buffer.
* Returns new number of bytes in buffer.
\*-------------------------------------------------------------------------*/
static size_t b64decode(UC c, UC *input, size_t size, 
        luaL_Buffer *buffer)
{
    /* ignore invalid characters */
    if (b64unbase[c] > 64) return size;
    input[size++] = c;
    /* decode atom */
    if (size == 4) {
        UC decoded[3];
        int valid, value = 0;
        value =  b64unbase[input[0]]; value <<= 6;
        value |= b64unbase[input[1]]; value <<= 6;
        value |= b64unbase[input[2]]; value <<= 6;
        value |= b64unbase[input[3]];
        decoded[2] = (UC) (value & 0xff); value >>= 8;
        decoded[1] = (UC) (value & 0xff); value >>= 8;
        decoded[0] = (UC) value;
        /* take care of paddding */
        valid = (input[2] == '=') ? 1 : (input[3] == '=') ? 2 : 3; 
        luaL_addlstring(buffer, (char *) decoded, valid);
        return 0;
    /* need more data */
    } else return size;
}

static int mydecodebase64 (lua_State *L)
  {
  UC atom[4];
  size_t asize = 0;

  // get text to hash
  size_t textLength;
  const UC * input = luaL_checklstring (L, 1, &textLength);
  const UC *last = input + textLength;    // where buffer ends

  // use a Lua buffer
  luaL_Buffer buffer;
  luaL_buffinit(L, &buffer);

  // process all input
  while (input < last) 
      asize = b64decode(*input++, atom, asize, &buffer);

  luaL_pushresult(&buffer);
  return 1;
  }

// make number table item
static void MakeNumberTableItem (lua_State *L, const char * name, const double n)
  {
  lua_pushstring (L, name);
  lua_pushnumber (L, n);
  lua_rawset(L, -3);
  }

// make boolean table item
static void MakeBoolTableItem (lua_State *L, const char * name, const int b)
  {
  if (b)
    {
    lua_pushstring (L, name);
    lua_pushboolean (L, b != 0);
    lua_rawset(L, -3);
    }
  }

static int getdirectory (lua_State *L)
  {
  // get directory name (eg. C:\mushclient\*.doc)
  size_t dirLength;
  const char * dirname = luaL_checklstring (L, 1, &dirLength);

  struct _finddatai64_t fdata;

  int h = _findfirsti64 (dirname, &fdata); // get handle

  if (h == -1L)    // no good?
    {
    lua_pushnil (L);

    switch (errno)
      {
      case EINVAL: lua_pushliteral (L, "Invalid filename specification"); break;
      default:     lua_pushliteral (L, "File specification could not be matched"); break;
      }
    return 2;   // return nil, error message
    }

  lua_newtable(L);    // table of entries
  
  do
    {

    lua_pushstring (L, fdata.name); // file name (will be key)
    lua_newtable(L);                // table of attributes

    // inside this new table put the file attributes

    MakeNumberTableItem (L, "size", (double) fdata.size);
    if (fdata.time_create != -1)    // except FAT
     MakeNumberTableItem (L, "create_time", fdata.time_create);
    if (fdata.time_access != -1)    // except FAT
      MakeNumberTableItem (L, "access_time", fdata.time_access);
    MakeNumberTableItem (L, "write_time",  fdata.time_write);
    MakeBoolTableItem   (L, "archive", fdata.attrib & _A_ARCH);
    MakeBoolTableItem   (L, "hidden", fdata.attrib & _A_HIDDEN);
    MakeBoolTableItem   (L, "normal", fdata.attrib & _A_NORMAL);
    MakeBoolTableItem   (L, "readonly", fdata.attrib & _A_RDONLY);
    MakeBoolTableItem   (L, "directory", fdata.attrib & _A_SUBDIR);
    MakeBoolTableItem   (L, "system", fdata.attrib & _A_SYSTEM);

    lua_rawset(L, -3);              // set key of table item (ie. file name)

    } while (_findnexti64 ( h, &fdata ) == 0);

  _findclose  (h);

  return 1;  // one table of entries
  } // end of getdirectory

// split routine suggested by Ked from the forum

static int l_split (lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  const char *sep = luaL_checkstring(L, 2);
  const int count = (int) luaL_optnumber (L, 3, 0);
  char *e;
  int i = 1;

  if (strlen (sep) != 1)
    luaL_error (L, "Separator must be a single character");

  if (count < 0)
    luaL_error (L, "Count must be positive or zero");

  lua_newtable(L);  /* result */

  /* repeat for each separator */
  while ((e = strchr(s, *sep)) != NULL && 
         (count == 0 || i <= count)) 
    {
    lua_pushlstring(L, s, e-s);  /* push substring */
    lua_rawseti(L, -2, i++);
    s = e + 1;  /* skip separator */
    }

  /* push last substring */
  lua_pushstring(L, s);
  lua_rawseti(L, -2, i);

  return 1;  /* return the table */
}
    
// table of operations
static const struct luaL_reg compresslib [] = 
  {

  {"base64decode", mydecodebase64},
  {"base64encode", myencodebase64},
  {"compress", mycompress},
  {"decompress", mydecompress},
  {"fromhex", utils_fromhex},
  {"hash", myhash},
  {"md5", utils_md5},
  {"readdir", getdirectory},
  {"sha256", utils_sha256},
  {"split", l_split},
  {"tohex", utils_tohex},

  {NULL, NULL}
  };

// register library

LUALIB_API int luaopen_compress(lua_State *L)
  {
  luaL_register (L, "utils", compresslib);
  b64setup ();
  return 1;
  }

