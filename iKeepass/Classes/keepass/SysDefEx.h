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

#ifndef ___SYS_DEF_EX_H___
#define ___SYS_DEF_EX_H___

/*
#ifndef CPP_FN_SHARE
	#define CPP_FN_SHARE 
#endif
*/


#ifdef CPP_CLASS_SHARE
#error CPP_CLASS_SHARE must not be defined.
#else
	/*
	#ifdef COMPILE_DLL_EX
		#define CPP_CLASS_SHARE __declspec(dllimport)
	#else
		#define CPP_CLASS_SHARE __declspec(dllexport)
	#endif
	*/
	#define CPP_CLASS_SHARE
#endif

#ifdef CPP_FN_SHARE
#error CPP_FN_SHARE must not be defined.
#else
	/*#ifdef COMPILE_DLL_EX
		#define CPP_FN_SHARE __declspec(dllimport)
	#else
		#define CPP_FN_SHARE __declspec(dllexport)
	#endif
	*/
	#define CPP_FN_SHARE
#endif

#ifdef C_FN_SHARE
#error C_FN_SHARE must not be defined.
#else
	/*
	#ifdef COMPILE_DLL_EX
		#define C_FN_SHARE extern "C" __declspec(dllimport)
	#else
		#define C_FN_SHARE extern "C" __declspec(dllexport)
	#endif
	*/
	#define C_FN_SHARE
#endif

#ifndef KP_EXP
	/*	
	#ifdef COMPILE_DLL_EX
		#define KP_EXP extern "C" __declspec(dllexport)
	#else
		#define KP_EXP extern "C" __declspec(dllimport)
	#endif
	*/
	#define KP_EXP
#endif

#ifndef KP_API
	//#define KP_API __cdecl
	#define KP_API
#endif

// Disable export warnings
//#pragma warning(disable: 4251)
//#pragma warning(disable: 4275)

/*
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(p) (void)0
#endif

#ifndef DWORD_MAX
#define DWORD_MAX 0xFFFFFFFF
#endif

#ifndef SHCNE_ASSOCCHANGED
#define SHCNE_ASSOCCHANGED 0x08000000L
#define SHCNF_IDLIST       0x0000
#endif
typedef void(WINAPI *LPSHCHANGENOTIFY)(LONG wEventId, UINT uFlags, LPCVOID dwItem1, LPCVOID dwItem2);
*/

#endif
