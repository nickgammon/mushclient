/*
* lbc.c
* big-number library for Lua 5.0 based on GNU bc-1.06 core library
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 02 Aug 2006 09:56:16
* This code is hereby placed in the public domain.
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "bcconfig.h"

#include "..\lua.h"
#include "..\lauxlib.h"

#include "number.h"

#ifndef lua_boxpointer
#define lua_boxpointer(L,u) \
	(*(void **)(lua_newuserdata(L, sizeof(void *))) = (u))
#define lua_unboxpointer(L,i)	(*(void **)(lua_touserdata(L, i)))
#endif

#define MYNAME		"bc"
#define MYVERSION	MYNAME " library for " LUA_VERSION " / Aug 2006 / "\
			"based on GNU bc-1.06"
#define MYTYPE		MYNAME " bignumber"

static int DIGITS;
static lua_State *LL=NULL;

void bc_error(char *mesg, ...)
{
 luaL_error(LL,"(bc) %s",mesg);
}

void bc_warn(char *mesg, ...)
{
#ifdef IGNORE_MAPM_WARNINGS
#else
 bc_error(mesg);
#endif
}

void bc_nomemory(void)
{
 bc_error("not enough memory");
}

static void Bnew(lua_State *L, bc_num x)
{
 lua_boxpointer(L,x);
 luaL_getmetatable(L,MYTYPE);
 lua_setmetatable(L,-2);
}

static bc_num Bget(lua_State *L, int i)
{
 LL=L;
 switch (lua_type(L,i))
 {
  case LUA_TNUMBER:
  case LUA_TSTRING:
  {
   bc_num x=NULL;
   const char *s=lua_tostring(L,i);
   for (; isspace(*s); s++);		/* bc_str2num chokes on spaces */
   bc_str2num(&x,(char*)s,DIGITS);
   Bnew(L,x);
   lua_replace(L,i);
   return x;
  }
  case LUA_TUSERDATA:
   if (luaL_checkudata(L,i,MYTYPE)!=NULL) return lua_unboxpointer(L,i);
 }
 luaL_typerror(L,i,MYTYPE);
 return NULL;
}

static int Bdo1(lua_State *L, void (*f)(bc_num a, bc_num b, bc_num *c, int n))
{
 bc_num a=Bget(L,1);
 bc_num b=Bget(L,2);
 bc_num c=NULL;
 f(a,b,&c,DIGITS);
 Bnew(L,c);
 return 1;
}

static int Bdo2(lua_State *L, int (*f)(bc_num a, bc_num b, bc_num *c, int n))
{
 bc_num a=Bget(L,1);
 bc_num b=Bget(L,2);
 bc_num c=NULL;
 if (f(a,b,&c,DIGITS)==0)
  Bnew(L,c);
 else
  lua_pushnil(L);
 return 1;
}

static int Bdigits(lua_State *L)		/** digits([n]) */
{
 lua_pushnumber(L,DIGITS);
 DIGITS=luaL_optint(L,1,DIGITS);
 return 1;
}

static int Bstring(lua_State *L)		/** tostring(x) */
{
 bc_num a=Bget(L,1);
 char *s=bc_num2str(a);
 lua_pushstring(L,s);
 free(s);
 return 1;
}

static int Biszero(lua_State *L)		/** iszero(x) */
{
 bc_num a=Bget(L,1);
 lua_pushboolean(L,bc_is_zero(a));
 return 1;
}

static int Bisneg(lua_State *L)			/** isneg(x) */
{
 bc_num a=Bget(L,1);
 lua_pushboolean(L,bc_is_neg(a));
 return 1;
}

static int Bnumber(lua_State *L)		/** number(x) */
{
 Bget(L,1);
 return 1;
}

static int Bcompare(lua_State *L)		/** compare(x,y) */
{
 bc_num a=Bget(L,1);
 bc_num b=Bget(L,2);
 lua_pushnumber(L,bc_compare(a,b));
 return 1;
}

static int Beq(lua_State *L)
{
 bc_num a=Bget(L,1);
 bc_num b=Bget(L,2);
 lua_pushboolean(L,bc_compare(a,b)==0);
 return 1;
}

static int Blt(lua_State *L)
{
 bc_num a=Bget(L,1);
 bc_num b=Bget(L,2);
 lua_pushboolean(L,bc_compare(a,b)<0);
 return 1;
}

static int Badd(lua_State *L)			/** add(x,y) */
{
 return Bdo1(L,bc_add);
}

static int Bsub(lua_State *L)			/** sub(x,y) */
{
 return Bdo1(L,bc_sub);
}

static int Bmul(lua_State *L)			/** mul(x,y) */
{
 return Bdo1(L,bc_multiply);
}

static int Bpow(lua_State *L)			/** pow(x,y) */
{
 return Bdo1(L,bc_raise);
}

static int Bdiv(lua_State *L)			/** div(x,y) */
{
 return Bdo2(L,bc_divide);
}

static int Bmod(lua_State *L)			/** mod(x,y) */
{
 return Bdo2(L,bc_modulo);
}

static int Bsqrt(lua_State *L)			/** sqrt(x) */
{
 bc_num a=Bget(L,1);
 bc_num b=bc_zero;
 bc_num c=NULL;
 bc_add(a,b,&c,DIGITS);				/* bc_sqrt works inplace! */
 if (bc_sqrt(&c,DIGITS)!=0)
  Bnew(L,c);
 else
  lua_pushnil(L);
 return 1;
}

static int Bgc(lua_State *L)
{
 bc_num a=Bget(L,1);
 bc_free_num(&a);
 return 0;
}

static int Bneg(lua_State *L)
{
 bc_num a=bc_zero;
 bc_num b=Bget(L,1);
 bc_num c=NULL;
 bc_sub(a,b,&c,DIGITS);
 Bnew(L,c);
 return 1;
}

static const luaL_reg R[] =
{
	{ "__add",	Badd	},
	{ "__div",	Bdiv	},
	{ "__eq",	Beq	},
	{ "__gc",	Bgc	},
	{ "__lt",	Blt	},
	{ "__mul",	Bmul	},
	{ "__pow",	Bpow	},
	{ "__sub",	Bsub	},
	{ "__tostring",	Bstring	},
	{ "__unm",	Bneg	},
	{ "add",	Badd	},
	{ "compare",	Bcompare},
	{ "digits",	Bdigits	},
	{ "div",	Bdiv	},
	{ "isneg",	Bisneg	},
	{ "iszero",	Biszero	},
	{ "mod",	Bmod	},
	{ "mul",	Bmul	},
	{ "number",	Bnumber	},
	{ "pow",	Bpow	},
	{ "sqrt",	Bsqrt	},
	{ "sub",	Bsub	},
	{ "tostring",	Bstring	},
	{ NULL,		NULL	}
};

LUALIB_API int luaopen_bc(lua_State *L)
{
 bc_init_numbers();
 lua_pushliteral(L,MYNAME);
 luaL_newmetatable(L,MYTYPE);
 luaL_openlib(L,NULL,R,0);
 lua_pushliteral(L,"version");			/** version */
 lua_pushliteral(L,MYVERSION);
 lua_settable(L,-3);
 lua_pushliteral(L,"__index");
 lua_pushvalue(L,-2);
 lua_settable(L,-3);
 lua_rawset(L,LUA_GLOBALSINDEX);
 return 1;
}
