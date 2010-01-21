/**************************************************************
 * MakewWindowTransparent.h
 * Copyright 2003, East Coast Toolworks LLC.
 *
 * by Mike Schaeffer
 * mschaef@mschaef.com
 *
 * Permission is freely granted to make use of this code as you
 * see fit. Nothing is guaranteed about this code and no warranties
 * are provided about suitability for any purpose.
 */

#ifndef __MAKEWINDOWTRANSPARENT_H
#define __MAKEWINDOWTRANSPARENT_H

#define MWT_MIN_FACTOR (0)
#define MWT_MAX_FACTOR (0xFF)

bool MakeWindowTransparent(HWND hWnd, long key, const unsigned char factor);
bool MakeWindowTransparent(CWnd *w, long key, const unsigned char factor);

#endif // __MAKEWINDOWTRANSPARENT_H