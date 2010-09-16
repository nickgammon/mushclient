#include "..\lua.h"
#include "..\lauxlib.h"

#include <ctype.h>
#include <stdlib.h>

// some bitwise operations for use in MUSHclient

/* Bitwise operations library */
/* Reuben Thomas   nov00-09jan04 */


#pragma warning( disable: 4057)  // 'const unsigned char *' differs in indirection to slightly different base types from 'const char *'
#pragma warning( disable: 4244)  // conversion from '__int64 ' to 'double ', possible loss of data
#pragma warning( disable: 4706)  // assignment within conditional expression

typedef __int64 Integer;
typedef unsigned __int64 UInteger;

#define luaL_checkbit(L, n)  ((Integer)luaL_checknumber(L, n))
#define luaL_checkubit(L, n) ((UInteger)luaL_checkbit(L, n))

// added cast to Integer to get rid of compiler error
#define TDYADIC(name, op, checkbit1, checkbit2) \
  static int bit_ ## name(lua_State* L) { \
    lua_pushnumber(L, \
      (Integer) (checkbit1(L, 1) op checkbit2(L, 2))); \
    return 1; \
  }

#define DYADIC(name, op) \
  TDYADIC(name, op, luaL_checkbit, luaL_checkbit)

#define MONADIC(name, op) \
  static int bit_ ## name(lua_State* L) { \
    lua_pushnumber(L, op luaL_checkbit(L, 1)); \
    return 1; \
  }

#define VARIADIC(name, op) \
  static int bit_ ## name(lua_State *L) { \
    int n = lua_gettop(L), i; \
    Integer w = luaL_checkbit(L, 1); \
    for (i = 2; i <= n; i++) \
      w op luaL_checkbit(L, i); \
    lua_pushnumber(L, w); \
    return 1; \
  }

MONADIC(bnot,     ~)
VARIADIC(band,    &=)
VARIADIC(bor,     |=)
VARIADIC(bxor,    ^=)
TDYADIC(lshift,  <<, luaL_checkbit, luaL_checkubit)
TDYADIC(rshift,  >>, luaL_checkubit, luaL_checkubit)
TDYADIC(arshift, >>, luaL_checkbit, luaL_checkubit)
DYADIC(mod,      %)

// convert string into an integral number in any base in range 2-36 (written by Nick Gammon)
// eg. n = bit.fromhex ("ABCDEF")
static int bit_tonumber (lua_State *L)
  {

  Integer result = 0;
  unsigned int base = luaL_optint(L, 2, 10);

  // get text to convert
  const unsigned char * text = luaL_checkstring (L, 1);
  const unsigned char * p;
  unsigned char c;
  unsigned int digit;
  Integer maxvalue;
  int negative = 0;

  if (base != 10)
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");

  // maximum value before multiply by the base
  maxvalue = 4503599627370496 / base;   // 2^52

  // skip whitespace
  for (p = text; isspace (*p); p++)
    ;  // skip leading spaces

  // look for sign
  if (*p == '-')
    {
    negative = 1;
    p++;
    }
  else if (*p == '+')
    p++;


  while (c = *p++)
    {

    // The largest mantissa a double can have it 52 bits
    if (result >= maxvalue) 
      luaL_error (L, "Number too big");

    if (isdigit (c))
      digit = c - '0';
    else if (isalpha (c))
      digit = toupper (c) - 'A' + 10;
    else
      digit = 999;   // bad digit - force error in next line

    if (digit >= base)
      luaL_error (L, "Bad digit ('%c') at position %d",
                  c, (int) (p - text));

    result = result * base + digit;
    } // end while

  if (negative)
    result = -result;

  lua_pushnumber (L, result);

  return 1;  // number of result fields
  } // end of bit_tonumber

// convert number to a string in any base in range 2-36 (written by Nick Gammon)
// eg. s = bit.tostring (281474976710656, 16)
static int bit_tostring (lua_State *L)
  {

  Integer n = luaL_checknumber (L, 1);
  UInteger un = n;
  int base = luaL_optint(L, 2, 10);
  char s [70] = "-";
  char *p = s;

  if (n < 0)
    {
    un = -n;
    p++;
    }

  if (base != 10)
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");

  // The largest mantissa a double can have it 52 bits
  if (un >= 4503599627370496)    // 2^52
    luaL_error (L, "Number too big");

  _ui64toa (un, p, base);

  // make upper case to be consistent with utils.tohex
  for (p = s; *p; p++) 
    *p = toupper (*p);

  lua_pushstring (L, s);

  return 1;  // number of result fields
  } // end of bit_tostring


static const struct luaL_reg bitlib[] = {

  {"ashr",  bit_arshift},      // was arshift in Reuben's library
  {"band",  bit_band},
  {"bor",   bit_bor},
  {"mod",   bit_mod},          // new
  {"neg",   bit_bnot},         // was bnot in Reuben's library
  {"shl",   bit_lshift},       // was lshift in Reuben's library
  {"shr",   bit_rshift},       // was rshift in Reuben's library
  {"tonumber", bit_tonumber},  // new by Nick
  {"tostring", bit_tostring},  // new by Nick
  {"xor",   bit_bxor},         // was bxor in Reuben's library

  {NULL, NULL}
};

// register library

LUALIB_API int luaopen_bits(lua_State *L)
  {
  luaL_register (L, "bit", bitlib);
  return 1;
  }


/*  OLD VERSION 
// shift right - shift arg1 right by arg2 bits
static int shr (lua_State *L)
  {
  unsigned long num = (unsigned long) luaL_checknumber (L, 1);
  unsigned long amount = (unsigned long) luaL_checknumber (L, 2);
  unsigned long result = num >> amount;
  lua_pushnumber (L, result);
  return 1;  // number of result fields
  } // end of shr

// shift left - shift arg1 left by arg2 bits
static int shl (lua_State *L)
  {
  unsigned long num = (unsigned long) luaL_checknumber (L, 1);
  unsigned long amount = (unsigned long) luaL_checknumber (L, 2);
  unsigned long result = num << amount;
  lua_pushnumber (L, result);
  return 1;  // number of result fields
  } // end of shl

// bitwise and ("and" is keyword) - takes multiple arguments
static int band (lua_State *L)
  {
  // first argument must be there or the result will be zero
  unsigned long result = (unsigned long) luaL_checknumber (L, 1);
  int n = lua_gettop(L);  // number of arguments 
  int i;
  for (i=2; i<=n; i++) 
    {
    unsigned long n = (unsigned long) luaL_checknumber (L, i);
    result &= n;
    }

  lua_pushnumber (L, result);
  return 1;  // number of result fields
  } // end of band

// bitwise or ("or" is keyword) - takes multiple arguments
static int bor (lua_State *L)
  {
  unsigned long result = 0;
  int n = lua_gettop(L);  // number of arguments 
  int i;
  for (i=1; i<=n; i++) 
    {
    unsigned long n = (unsigned long) luaL_checknumber (L, i);
    result |= n;
    }

  lua_pushnumber (L, result);
  return 1;  // number of result fields
  } // end of bor

// xor - takes multiple arguments
static int xor (lua_State *L)
  {
  unsigned long result = 0;
  int n = lua_gettop(L);  // number of arguments 
  int i;
  for (i=1; i<=n; i++) 
    {
    unsigned long n = (unsigned long) luaL_checknumber (L, i);
    result ^= n;
    }

  lua_pushnumber (L, result);
  return 1;  // number of result fields
  } // end of xor

// neg - bitwise negate (ones complement)
static int neg (lua_State *L)
  {
  unsigned long num = (unsigned long) luaL_checknumber (L, 1);
  unsigned long result = ~num;
  lua_pushnumber (L, result);
  return 1;  // number of result fields
  } // end of neg

// table of operations
static const struct luaL_reg bitslib [] = 
  {

  {"shr", shr},
  {"shl", shl},
  {"band", band},
  {"bor", bor},
  {"xor", xor},
  {"neg", neg},

  {NULL, NULL}
  };


// register library

LUALIB_API int luaopen_bits(lua_State *L)
  {
  luaL_register (L, "bit", bitslib);
  return 1;
  }

  END OLD VERSION */

