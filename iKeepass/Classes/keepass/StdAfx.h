/*
  KeePass Password Safe - The Open-Source Password Manager
  Copyright (C) 2003-2005 Dominik Reichl <dominik.reichl@t-online.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(AFX_STDAFX_H__206CC2C5_063D_11D8_BF16_0050BF14F5CC__INCLUDED_)
#define AFX_STDAFX_H__206CC2C5_063D_11D8_BF16_0050BF14F5CC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

//#include <afxwin.h>
//#include <Tchar.h>

#include <stdlib.h>
#include <string.h>


#define ASSERT(f) \
	do \
	{ \
	if (!(f)) ; \
	} while (0) \


// Standard constants
#undef FALSE
#undef TRUE
#undef NULL

#define FALSE   0
#define TRUE    1
#define NULL    0

//#include <afxext.h>
//#include <afxmt.h>
//#ifndef _AFX_NO_AFXCMN_SUPPORT
//#include <afxcmn.h>
//#endif // _AFX_NO_AFXCMN_SUPPORT

// #define CR_BACK RGB(216,216,235) /* Blue */
// #define CR_BACK RGB(240,236,224) /* WinXP */
//#define CR_BACK RGB(208,208,208) /* Win2k */

//#define CR_FRONT RGB(0,0,0)

//#include <afxdisp.h>
//#include <afxole.h>

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0500

typedef char TCHAR, *PTCHAR;
typedef unsigned char TBYTE , *PTBYTE ;


typedef unsigned char       BYTE;
typedef unsigned long       DWORD;

//#ifndef BOOL
	//typedef int                 BOOL;
	typedef signed char		BOOL;
//#endif

typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;

typedef int                 INT;
typedef unsigned int        UINT;

typedef unsigned short		USHORT;

typedef char CHAR;
typedef short SHORT;
typedef long LONG;


#ifndef CONST
	#define CONST               const
#endif

typedef CHAR *LPSTR, *PSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;
typedef LPCSTR LPCTSTR;
typedef LPSTR PTSTR, LPTSTR;

#define __T(x)      x
#define _T(x)       __T(x)
#define _TEXT(x)    __T(x)

#define _tfopen     fopen

#define _PUC    unsigned char *
#define _CPUC   const unsigned char *
#define _PC     char *
#define _CPC    const char *
#define _UI     unsigned int

//__inline int _tcsicmp(_CPC _s1,_CPC _s2) {return _mbsicmp((_CPUC)_s1,(_CPUC)_s2);}
//__inline int _tcsicmp(_CPC _s1,_CPC _s2) {return strcmp((_CPUC)_s1,(_CPUC)_s2);}
// hier eigentlich strcmpi !!!
__inline int _tcsicmp(_CPC _s1,_CPC _s2) {return strcmp(_s1,_s2);}

//__inline int _tcscmp(_CPC _s1,_CPC _s2) {return _mbscmp((_CPUC)_s1,(_CPUC)_s2);}
__inline int _tcscmp(_CPC _s1,_CPC _s2) {return strcmp(_s1,_s2);}

#define _tcscat     strcat
#define _tcscpy     strcpy
#define _tcsdup     _strdup

#define _tcslen     strlen

//#define _itot       _itoa
#define _itot       itoa

#define _ttoi       atoi
#define _ttol       atol

#define _stprintf   sprintf


//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_STDAFX_H__206CC2C5_063D_11D8_BF16_0050BF14F5CC__INCLUDED_)
