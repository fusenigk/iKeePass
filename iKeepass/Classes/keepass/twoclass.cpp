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
#include "twoclass.h"
#include <string.h>


static bool g_bInitialized = false;

CTwofish::CTwofish()
{
}

CTwofish::~CTwofish()
{
}

bool CTwofish::init(RD_UINT8 *pKey, unsigned long uKeyLen, RD_UINT8 *initVector)
{
	ASSERT(pKey != NULL);
	if(pKey == NULL) return false;
	ASSERT(uKeyLen != 0);
	if(uKeyLen == 0) return false;

	if(g_bInitialized == false)
	{
		Twofish_initialise();
		g_bInitialized = true;
	}

	Twofish_prepare_key((Twofish_Byte *)pKey, uKeyLen, &m_key);

	if(initVector != NULL) memcpy(m_pInitVector, initVector, 16);
	else memset(m_pInitVector, 0, 16);

	return true;
}

int CTwofish::padEncrypt(RD_UINT8 *pInput, int nInputOctets, RD_UINT8 *pOutBuffer)
{
	int i, numBlocks, padLen;
	RD_UINT8 block[16], *iv;

	ASSERT((pInput != NULL) && (nInputOctets != NULL) && (pOutBuffer != NULL));
	if((pInput == NULL) || (nInputOctets <= 0) || (pOutBuffer == NULL)) return 0;

	numBlocks = nInputOctets / 16;

	iv = m_pInitVector;
	for(i = numBlocks; i > 0; i--)
	{
		((RD_UINT32*)block)[0] = ((RD_UINT32*)pInput)[0] ^ ((RD_UINT32*)iv)[0];
		((RD_UINT32*)block)[1] = ((RD_UINT32*)pInput)[1] ^ ((RD_UINT32*)iv)[1];
		((RD_UINT32*)block)[2] = ((RD_UINT32*)pInput)[2] ^ ((RD_UINT32*)iv)[2];
		((RD_UINT32*)block)[3] = ((RD_UINT32*)pInput)[3] ^ ((RD_UINT32*)iv)[3];

		Twofish_encrypt(&m_key, (Twofish_Byte *)block, (Twofish_Byte *)pOutBuffer);

		iv = pOutBuffer;
		pInput += 16;
		pOutBuffer += 16;
	}

	padLen = 16 - (nInputOctets - (16 * numBlocks));

	for (i = 0; i < 16 - padLen; i++)
	{
		block[i] = (RD_UINT8)(pInput[i] ^ iv[i]);
	}

	for (i = 16 - padLen; i < 16; i++)
	{
		block[i] = (RD_UINT8)((RD_UINT8)padLen ^ iv[i]);
	}

	Twofish_encrypt(&m_key, (Twofish_Byte *)block, (Twofish_Byte *)pOutBuffer);

	return 16 * (numBlocks + 1);
}

int CTwofish::padDecrypt(RD_UINT8 *pInput, int nInputOctets, RD_UINT8 *pOutBuffer)
{
	int i, numBlocks, padLen;
	RD_UINT8 block[16];
	RD_UINT32 iv[4];

	ASSERT((pInput != NULL) && (nInputOctets != NULL) && (pOutBuffer != NULL));
	if((pInput == NULL) || (nInputOctets <= 0) || (pOutBuffer == NULL)) return 0;

	if((nInputOctets % 16) != 0) { ASSERT(FALSE); return -1; }

	numBlocks = nInputOctets / 16;

	memcpy(iv, m_pInitVector, 16);

	for(i = numBlocks - 1; i > 0; i--)
	{
		Twofish_decrypt(&m_key, (Twofish_Byte *)pInput, (Twofish_Byte *)block);
		((RD_UINT32*)block)[0] ^= iv[0];
		((RD_UINT32*)block)[1] ^= iv[1];
		((RD_UINT32*)block)[2] ^= iv[2];
		((RD_UINT32*)block)[3] ^= iv[3];
		memcpy(iv, pInput, 16);
		memcpy(pOutBuffer, block, 16);
		pInput += 16;
		pOutBuffer += 16;
	}

	Twofish_decrypt(&m_key, (Twofish_Byte *)pInput, (Twofish_Byte *)block);
	((RD_UINT32*)block)[0] ^= iv[0];
	((RD_UINT32*)block)[1] ^= iv[1];
	((RD_UINT32*)block)[2] ^= iv[2];
	((RD_UINT32*)block)[3] ^= iv[3];
	padLen = block[15];
	if(padLen <= 0 || padLen > 16) return -1;
	for(i = 16 - padLen; i < 16; i++)
	{
		if(block[i] != padLen) return -1;
	}
	memcpy(pOutBuffer, block, 16 - padLen);

	return 16*numBlocks - padLen;
}
