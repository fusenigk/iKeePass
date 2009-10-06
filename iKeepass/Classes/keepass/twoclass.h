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

#ifndef ___TWOFISH_CLASS_H___
#define ___TWOFISH_CLASS_H___

#include "twofish.h"
#include "rijndael.h"
#include "SysDefEx.h"

class CTwofish
{
public:
	CTwofish();
	virtual ~CTwofish();

	bool init(RD_UINT8 *pKey, unsigned long uKeyLen, RD_UINT8 *initVector = NULL);

	int padEncrypt(RD_UINT8 *pInput, int nInputOctets, RD_UINT8 *pOutBuffer);
	int padDecrypt(RD_UINT8 *pInput, int nInputOctets, RD_UINT8 *pOutBuffer);

private:
	Twofish_key m_key;
	RD_UINT8 m_pInitVector[16];
};

#endif
