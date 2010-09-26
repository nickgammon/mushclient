/* Visual C++ 6 does not have support for the uxtheme library, nor
 * is MUSHclient willing to drop support for pre-XP platforms.
 * Thus, we need an 'optional' alternative to the uxtheme.lib
 * library.
 * 
 * Sadly, stdafx.h includes a whole lot of MFC libraries,
 * which include uxtheme.h on their own, yet do not seem to use it.
 *
 * Thus, I cannot (easily) create dynamically linked functions to
 * interface with uxtheme.dll as they are already defined, yet using
 * the already defined uxtheme.h functions requires linking against
 * uxtheme.lib. -JW
 */


#ifndef __THEMEGLUE_H__
#define __THEMEGLUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

HMODULE HModUXT = NULL;

#define THEMEGLUE_FUNC(FuncStr, params, result, FuncName, formal_params) \
result FuncName formal_params  \
{   \
	/* Gotta load UxTheme.dll to find the function we want. */ \
	if (HModUXT == NULL)  \
		HModUXT = LoadLibrary("UxTheme.dll"); \
	\
	if (HModUXT != NULL)  \
	{ \
		typedef result (WINAPI *FuncPtr) formal_params ; \
		FuncPtr FuncCall = (FuncPtr) GetProcAddress(HModUXT, FuncStr); \
		if (FuncCall != NULL) \
			return FuncCall params ; \
	} \
	/* No real failure possible, all is optional. */ \
	return S_OK ; \
}


/* THEMEGLUE_FUNC Format is: EntryPointString, SuppliedParameters, ResultType, FunctionName, FormalParameters */

THEMEGLUE_FUNC("EnableThemeDialogTexture", (hwnd, dwFlags), HRESULT, ThemeGlue_EnableThemeDialogTexture, (HWND hwnd, DWORD dwFlags))


/* 2010-09-26: Yes, all this work for a single function. =)
 * But we might need more in the future, so I made it relatively easy to expand upon.
 * TODO: support for static linking against uxtheme.lib, but ideally we refactor
 *       stdafx.h a bit and get rid of that include alltogether.
 * -JW
 */


#ifndef _UXTHEME_H_
/* We need some of these with EnableThemeDialogTexture, make sure we have them. */
#define ETDT_DISABLE                    0x00000001
#define ETDT_ENABLE                     0x00000002
#define ETDT_USETABTEXTURE              0x00000004
#define ETDT_ENABLETAB              (ETDT_ENABLE | ETDT_USETABTEXTURE)
#endif

#ifdef __cplusplus
}   // extern "C"
#endif

#endif