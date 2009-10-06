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

#include "StdAfx.h"
#include "MemUtil.h"
#include "NewRandom.h"
#include "sha256.h"
#include "stdio.h"
//#include "../Crypto/sha2.h"

C_FN_SHARE void mem_erase(unsigned char *p, unsigned long u)
{
	unsigned long i;

	ASSERT(sizeof(unsigned char) == 1);
	ASSERT(p != NULL); if(p == NULL) return;
	if(u == 0) return;

	for(i = 0; i < u; i++)
		p[i] = (unsigned char)(rand() & 0xFF);

	memset(p, 0, u);
}

/*
#ifndef _WIN32_WCE
C_FN_SHARE BOOL SecureDeleteFile(LPCSTR pszFilePath)
{
	HANDLE hFile;
	DWORD i, m, dwSizeLo, dwTmp;
	int n;
	BYTE *pBuf;
	BOOL bRet;

	hFile = CreateFile(pszFilePath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE) return FALSE;

	dwSizeLo = GetFileSize(hFile, NULL);
	if(dwSizeLo == 0) { CloseHandle(hFile); return DeleteFile(pszFilePath); }

	SetFilePointer(hFile, 0, NULL, FILE_END);
	WriteFile(hFile, &i, 4, &dwTmp, NULL);
	FlushFileBuffers(hFile);

	pBuf = new BYTE[SDF_BUF_SIZE];
	if(pBuf == NULL) { CloseHandle(hFile); return FALSE; }

	// Four rounds: zeros, ones, random, random
	for(n = 0; n < 4; n++)
	{
		if(n == 0) memset(pBuf, 0, SDF_BUF_SIZE);
		else if(n == 1) memset(pBuf, 0xFF, SDF_BUF_SIZE);
		else
		{
			for(i = 0; i < SDF_BUF_SIZE; i++)
				pBuf[i] = (BYTE)randXorShift();
		}

		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		i = dwSizeLo + (DWORD)n; // Each round one more character at the end

		while(i != 0)
		{
			m = (i < SDF_BUF_SIZE) ? i : SDF_BUF_SIZE;

			bRet = WriteFile(hFile, pBuf, m, &dwTmp, NULL);
			FlushFileBuffers(hFile);

			ASSERT(bRet == TRUE);
			if(bRet != TRUE) break;
			if(m != dwTmp) { ASSERT(FALSE); break; }

			i -= m;
		}
	}

	CloseHandle(hFile);
	SAFE_DELETE_ARRAY(pBuf);

	if(DeleteFile(pszFilePath) == FALSE)
		if(remove(pszFilePath) != 0)
			return FALSE;

	return TRUE;
}
#endif
*/

// Pack time to 5 byte structure:
// Byte bits: 11111111 22222222 33333333 44444444 55555555
// Contents : 00YYYYYY YYYYYYMM MMDDDDDH HHHHMMMM MMSSSSSS

C_FN_SHARE void _PackTimeToStruct(BYTE *pBytes, DWORD dwYear, DWORD dwMonth, DWORD dwDay, DWORD dwHour, DWORD dwMinute, DWORD dwSecond)
{
	ASSERT(pBytes != NULL); if(pBytes == NULL) return;
	// Pack the time to a 5 byte structure
	pBytes[0] = (BYTE)((dwYear >> 6) & 0x0000003F);
	pBytes[1] = (BYTE)(((dwYear & 0x0000003F) << 2) | ((dwMonth >> 2) & 0x00000003));
	pBytes[2] = (BYTE)(((dwMonth & 0x00000003) << 6) | ((dwDay & 0x0000001F) << 1) | ((dwHour >> 4) & 0x00000001));
	pBytes[3] = (BYTE)(((dwHour & 0x0000000F) << 4) | ((dwMinute >> 2) & 0x0000000F));
	pBytes[4] = (BYTE)(((dwMinute & 0x00000003) << 6) | (dwSecond & 0x0000003F));
}

C_FN_SHARE void _UnpackStructToTime(BYTE *pBytes, DWORD *pdwYear, DWORD *pdwMonth, DWORD *pdwDay, DWORD *pdwHour, DWORD *pdwMinute, DWORD *pdwSecond)
{
	DWORD dw1, dw2, dw3, dw4, dw5;

	ASSERT(pBytes != NULL); if(pBytes == NULL) return;

	dw1 = (DWORD)pBytes[0]; dw2 = (DWORD)pBytes[1]; dw3 = (DWORD)pBytes[2];
	dw4 = (DWORD)pBytes[3]; dw5 = (DWORD)pBytes[4];

	// Unpack 5 byte structure to date and time
	*pdwYear = (dw1 << 6) | (dw2 >> 2);
	*pdwMonth = ((dw2 & 0x00000003) << 2) | (dw3 >> 6);
	*pdwDay = (dw3 >> 1) & 0x0000001F;
	*pdwHour = ((dw3 & 0x00000001) << 4) | (dw4 >> 4);
	*pdwMinute = ((dw4 & 0x0000000F) << 2) | (dw5 >> 6);
	*pdwSecond = dw5 & 0x0000003F;
}

C_FN_SHARE void _GetCurrentPwTime(PW_TIME *p)
{
	/*
	SYSTEMTIME t;
	ASSERT(p != NULL);
	GetLocalTime(&t);
	p->btDay = (BYTE)t.wDay; p->btHour = (BYTE)t.wHour;
	p->btMinute = (BYTE)t.wMinute; p->btMonth = (BYTE)t.wMonth;
	p->btSecond = (BYTE)t.wSecond; p->shYear = (USHORT)t.wYear;
	*/
}

C_FN_SHARE int _pwtimecmp(const PW_TIME *pt1, const PW_TIME *pt2)
{
	if(pt1->shYear < pt2->shYear) return -1;
	else if(pt1->shYear > pt2->shYear) return 1;

	if(pt1->btMonth < pt2->btMonth) return -1;
	else if(pt1->btMonth > pt2->btMonth) return 1;

	if(pt1->btDay < pt2->btDay) return -1;
	else if(pt1->btDay > pt2->btDay) return 1;

	if(pt1->btHour < pt2->btHour) return -1;
	else if(pt1->btHour > pt2->btHour) return 1;

	if(pt1->btMinute < pt2->btMinute) return -1;
	else if(pt1->btMinute > pt2->btMinute) return 1;

	if(pt1->btSecond < pt2->btSecond) return -1;
	else if(pt1->btSecond > pt2->btSecond) return 1;

	return 0; // They are exactly the same
}

/**
// kf
// itoa Aufruf durch sprintf ersetzen //
// Packs an array of integers to a TCHAR string
C_FN_SHARE void ar2str(TCHAR *tszString, INT *pArray, INT nItemCount)
{
	INT i;
	TCHAR tszTemp[20];

	ASSERT(tszString != NULL); if(tszString == NULL) return;
	ASSERT(pArray != NULL); if(pArray == NULL) return;

	tszString[0] = 0;
	if(nItemCount == 0) return;

	_itot(pArray[0], tszString, 10);

	for(i = 1; i < nItemCount; i++)
	{
		_tcscat(tszString, _T(" "));
		_itot(pArray[i], tszTemp, 10);
		_tcscat(tszString, tszTemp);
	}
}
*/

// Unpacks a TCHAR string to an array of integers
C_FN_SHARE void str2ar(TCHAR *tszString, INT *pArray, INT nItemCount)
{
	INT i = 0;
	TCHAR *p = tszString;
	TCHAR *lastp = tszString;

	ASSERT(tszString != NULL); if(tszString == NULL) return;
	ASSERT(pArray != NULL); if(pArray == NULL) return;

	lastp--;
	while(1)
	{
		if((*p == _T(' ')) || (*p == 0))
		{
			lastp++;
			pArray[i] = _ttoi(lastp);
			lastp = p;
			i++;
		}
		if(i == nItemCount) break;

		p++;
	}
}

C_FN_SHARE BOOL SHA256_HashFile(LPCTSTR lpFile, BYTE *pHash)
{
	FILE *fp = NULL;
	unsigned char *pBuf = NULL;
	size_t nRead = 0;

	// kf replaced sha256 class
	//	sha256_ctx sha32;
	sha256_context sha32 ;

	ASSERT(lpFile != NULL); if(lpFile == NULL) return FALSE;
	ASSERT(pHash != NULL); if(pHash == NULL) return FALSE;

	pBuf = new unsigned char[1024];
	ASSERT(pBuf != NULL); if(pBuf == NULL) return FALSE;

	fp = _tfopen(lpFile, _T("rb"));
	if(fp == NULL) return FALSE;

	// kf 
	sha256_starts(&sha32);
//	sha256_begin(&sha32);

	while(1)
	{
		nRead = fread(pBuf, 1, 1024, fp);
		if(nRead == 0) break;

		sha256_update(&sha32, pBuf, (unsigned long)nRead);
		//sha256_hash(pBuf, (unsigned long)nRead, &sha32);

		if(nRead != 1024) break;
	}

	sha256_finish(&sha32, (unsigned char *)pHash);
	//sha256_end((unsigned char *)pHash, &sha32);

	fclose(fp); fp = NULL;

	SAFE_DELETE_ARRAY(pBuf);
	return TRUE;
}
