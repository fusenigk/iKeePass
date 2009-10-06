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

#ifndef ___MEMORY_UTILITIES_H___
#define ___MEMORY_UTILITIES_H___

#include "StdAfx.h"
#include "PwManager.h"

// Safely delete pointers
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if((p) != NULL) { delete (p); (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p) { if((p) != NULL) { delete [](p); (p) = NULL; } }
#define SAFE_RELEASE(p)      { if((p) != NULL) { (p)->Release(); (p) = NULL; } }
#endif

// Maximum temporary buffer for SecureDeleteFile
#define SDF_BUF_SIZE 4096

// Securely erase memory
C_FN_SHARE void mem_erase(unsigned char *p, unsigned long u);

//#ifndef _WIN32_WCE
//C_FN_SHARE BOOL SecureDeleteFile(LPCSTR pszFilePath);
//#endif

// Time conversion functions
C_FN_SHARE void _PackTimeToStruct(BYTE *pBytes, DWORD dwYear, DWORD dwMonth, DWORD dwDay, DWORD dwHour, DWORD dwMinute, DWORD dwSecond);
C_FN_SHARE void _UnpackStructToTime(BYTE *pBytes, DWORD *pdwYear, DWORD *pdwMonth, DWORD *pdwDay, DWORD *pdwHour, DWORD *pdwMinute, DWORD *pdwSecond);

// Getting the time
C_FN_SHARE void _GetCurrentPwTime(PW_TIME *p);

// Compare two PW_TIME structures, returns -1 if pt1<pt2, returns 1 if pt1>pt2,
// returns 0 if pt1=pt2
C_FN_SHARE int _pwtimecmp(const PW_TIME *pt1, const PW_TIME *pt2);

// Packs an array of integers to a TCHAR string
C_FN_SHARE void ar2str(TCHAR *tszString, INT *pArray, INT nItemCount);

// Unpacks a TCHAR string to an array of integers
C_FN_SHARE void str2ar(TCHAR *tszString, INT *pArray, INT nItemCount);

// Hash a file
C_FN_SHARE BOOL SHA256_HashFile(LPCTSTR lpFile, BYTE *pHash);

#endif
