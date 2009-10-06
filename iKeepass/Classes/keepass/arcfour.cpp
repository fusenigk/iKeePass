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
#include "arcfour.h"

C_FN_SHARE void arcfourCrypt(BYTE *pBuf, unsigned long uBufLen, BYTE *pKey, unsigned long uKeyLen)
{
	BYTE S[256];
	BYTE i, j;
	BYTE t;
	DWORD w, k;

	ASSERT((sizeof(BYTE) == 1) && (pBuf != NULL) && (pKey != NULL) && (uKeyLen != 0));
	////kf ASSERT((IsBadWritePtr(pBuf, 1) == FALSE) && (IsBadReadPtr(pKey, 1) == FALSE));

	for(w = 0; w < 256; w++) S[w] = (BYTE)w; // Fill linearly

	i = 0; j = 0; k = 0;
	for(w = 0; w < 256; w++) // Key setup
	{
#pragma warning(push)
#pragma warning(disable: 4244)
		j += S[w] + (BYTE)(pKey[k] + (BYTE)((uBufLen & 0xFF) << 2));
#pragma warning(pop)

		t = S[i];
		S[i] = S[j];
		S[j] = t;

		k++;
		if(k == uKeyLen) k = 0;
	}

	i = 0; j = 0;

	for(w = 0; w < uBufLen; w++) // Generate random bytes and XOR with PT
	{
		i++;
#pragma warning(push)
#pragma warning(disable: 4244)
		j += S[i];
#pragma warning(pop)

		t = S[i];
		S[i] = S[j];
		S[j] = t;

		t = (BYTE)(S[i] + S[j]);
		pBuf[w] ^= S[t];
	}
}
