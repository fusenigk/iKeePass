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
#include "PwManager.h"
//#include "PwUtil.h"
#include "MemUtil.h"
//#include "../Util/StrUtil.h"
#include "twoclass.h"
//#include "../Crypto/sha2.h"
#include "sha256.h"
#include "arcfour.h"
#include "stdio.h"

//#include "../NewGUI/TranslateEx.h"

static const BYTE g_uuidZero[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
static PW_TIME g_pwTimeNever = { 2999, 12, 28, 23, 59, 59 };
static char g_pNullString[4] = { 0, 0, 0, 0 };

CPwManager::CPwManager()
{
	m_pEntries = NULL;
	m_dwNumEntries = 0;
	m_dwMaxEntries = 0;

	m_pGroups = NULL;
	m_dwNumGroups = 0;
	m_dwMaxGroups = 0;

	m_pLastEditedEntry = NULL;
	m_nAlgorithm = ALGO_AES;
	m_dwKeyEncRounds = PWM_STD_KEYENCROUNDS;

	memset(m_pMasterKey, 0, 32);

	_AllocGroups(PWM_NUM_INITIAL_GROUPS);
	_AllocEntries(PWM_NUM_INITIAL_ENTRIES);

	m_random.Initialize();
	////!!!m_random.GetRandomBuffer(m_pSessionKey, PWM_SESSION_KEY_SIZE);

	DWORD dwInitXorShift[4];
	///!!!m_random.GetRandomBuffer((BYTE *)&dwInitXorShift, 16);
	srandXorShift(dwInitXorShift);

	ASSERT(sizeof(BYTE) == 1);
}

CPwManager::~CPwManager()
{
	CleanUp();
}

void CPwManager::CleanUp()
{
	if(m_pEntries != NULL) _DeleteEntryList(TRUE);
	m_dwNumEntries = 0;
	m_dwMaxEntries = 0;

	if(m_pGroups != NULL) _DeleteGroupList(TRUE);
	m_dwNumGroups = 0;
	m_dwMaxGroups = 0;

	m_pLastEditedEntry = NULL;

	mem_erase(m_pMasterKey, 32);

	m_random.Reset();
}

int CPwManager::SetMasterKey(const TCHAR *pszMasterKey, BOOL bDiskDrive, const TCHAR *pszSecondKey, const CNewRandomInterface *pARI, BOOL bOverwrite)
{
	unsigned long uKeyLen, uKeyLen2 = 0, uFileSize, uRead;
	TCHAR szFile[2048];
	
	// kf 
	///sha256_ctx sha32;
	sha256_context sha32 ;

	char *paKey = NULL;
	char *paKey2 = NULL;
	unsigned char aFileKey[32];
	unsigned char aPasswordKey[32];
	BOOL bReadNormal;

	ASSERT(pszMasterKey != NULL); if(pszMasterKey == NULL) return PWE_INVALID_PARAM;

#ifdef _UNICODE
	ASSERT(sizeof(TCHAR) >= 2);
	paKey = _StringToAnsi(pszMasterKey);
#else
	ASSERT(sizeof(TCHAR) == 1);
	paKey = new char[strlen(pszMasterKey) + 1];
	ASSERT(paKey != NULL); if(paKey == NULL) return PWE_NO_MEM;
	strcpy(paKey, pszMasterKey);
#endif

	ASSERT(paKey != NULL); if(paKey == NULL) return PWE_NO_MEM;

	if(pszSecondKey != NULL)
	{
#ifdef _UNICODE
		ASSERT(sizeof(TCHAR) >= 2);
		paKey2 = _StringToAnsi(pszSecondKey);
#else
		ASSERT(sizeof(TCHAR) == 1);
		paKey2 = new char[strlen(pszSecondKey) + 1];
		ASSERT(paKey2 != NULL); if(paKey2 == NULL) return PWE_NO_MEM;
		strcpy(paKey2, pszSecondKey);
#endif

		ASSERT(paKey2 != NULL); if(paKey2 == NULL) return PWE_NO_MEM;

		uKeyLen2 = strlen(paKey2);
		ASSERT(uKeyLen2 != 0);
	}

	uKeyLen = strlen(paKey);
	ASSERT(uKeyLen != 0);
	if(uKeyLen == 0) { SAFE_DELETE_ARRAY(paKey); return PWE_INVALID_KEY; }

//	if(bDiskDrive == FALSE)
//	{
		sha256_starts(&sha32);
		sha256_update(&sha32, (unsigned char *)paKey, uKeyLen);
		sha256_finish(&sha32, (unsigned char *)m_pMasterKey);

		// kf 
		///sha256_begin(&sha32);
		///sha256_hash((unsigned char *)paKey, uKeyLen, &sha32);
		///sha256_end((unsigned char *)m_pMasterKey, &sha32);

		mem_erase((unsigned char *)paKey, uKeyLen);
		SAFE_DELETE_ARRAY(paKey);
		return PWE_SUCCESS;
/*	}
	else
	{
		if(pszSecondKey == NULL)
		{
			mem_erase((unsigned char *)paKey, uKeyLen);
			SAFE_DELETE_ARRAY(paKey); // Don't need ASCII key any more from on now

			_tcscpy(szFile, pszMasterKey);
			if(szFile[_tcslen(szFile) - 1] == _T('\\'))
			{
				_tcscat(szFile, PWS_DEFAULT_KEY_FILENAME);
			}

			if(pARI == NULL) // If pARI is NULL: load key from disk
			{
				FILE *fp;

				fp = _tfopen(szFile, _T("rb"));
				if(fp == NULL) return PWE_NOFILEACCESS_READ;
				fseek(fp, 0, SEEK_END);
				uFileSize = (unsigned long)ftell(fp);
				fseek(fp, 0, SEEK_SET);

				bReadNormal = TRUE;

				if(uFileSize == 32)
				{
					if(fread(m_pMasterKey, 1, 32, fp) != 32)
					{
						fclose(fp); fp = NULL;
						return PWE_FILEERROR_READ;
					}

					bReadNormal = FALSE;
				}
				else if(uFileSize == 64)
				{
					if(LoadHexKey32(fp, m_pMasterKey) == FALSE)
					{
						fseek(fp, 0, SEEK_SET);
					}
					else bReadNormal = FALSE;
				}

				if(bReadNormal == TRUE)
				{
					sha256_begin(&sha32);
					while(1)
					{
						uRead = (unsigned long)fread((unsigned char *)szFile, 1, 2048, fp);
						if(uRead == 0) break;
						sha256_hash((unsigned char *)szFile, uRead, &sha32);
						if(uRead != 2048) break;
					}
					sha256_end((unsigned char *)m_pMasterKey, &sha32);
				}

				fclose(fp); fp = NULL;
				return PWE_SUCCESS;
			}
			else // pARI is not NULL: save key to disk
			{
				FILE *fp;
				unsigned char aRandomBytes[32];

				fp = _tfopen(szFile, _T("rb")); // Does the file exist already?
				if((fp != NULL) && (bOverwrite == FALSE)) { fclose(fp); fp = NULL; return PWE_NOFILEACCESS_READ; }
				if(fp != NULL) { fclose(fp); fp = NULL; } // We must close it before opening for write

				if(pARI->GenerateRandomSequence(32, aRandomBytes) == FALSE) return PWE_INVALID_RANDOMSOURCE;

				fp = _tfopen(szFile, _T("wb"));
				if(fp == NULL) return PWE_NOFILEACCESS_WRITE;
				if(SaveHexKey32(fp, aRandomBytes) == FALSE) { fclose(fp); fp = NULL; return PWE_FILEERROR_WRITE; }
				fclose(fp); fp = NULL;

				memcpy(m_pMasterKey, aRandomBytes, 32);
				return PWE_SUCCESS;
			}
		}
		else // pszSecondKey != NULL
		{
			mem_erase((unsigned char *)paKey, uKeyLen);
			SAFE_DELETE_ARRAY(paKey); // Don't need ASCII key any more from on now

			_tcscpy(szFile, pszMasterKey);
			if(szFile[_tcslen(szFile) - 1] == _T('\\'))
			{
				_tcscat(szFile, PWS_DEFAULT_KEY_FILENAME);
			}

			if(pARI == NULL) // If pARI is NULL: load key from disk
			{
				FILE *fp;

				fp = _tfopen(szFile, _T("rb"));
				if(fp == NULL) return PWE_NOFILEACCESS_READ;
				fseek(fp, 0, SEEK_END);
				uFileSize = (unsigned long)ftell(fp);
				fseek(fp, 0, SEEK_SET);

				bReadNormal = TRUE;

				if(uFileSize == 32)
				{
					if(fread(aFileKey, 1, 32, fp) != 32)
					{
						fclose(fp); fp = NULL;
						return PWE_FILEERROR_READ;
					}

					bReadNormal = FALSE;
				}
				else if(uFileSize == 64)
				{
					if(LoadHexKey32(fp, aFileKey) == FALSE)
					{
						fseek(fp, 0, SEEK_SET);
					}
					else bReadNormal = FALSE;
				}

				if(bReadNormal == TRUE)
				{
					sha256_begin(&sha32);
					while(1)
					{
						uRead = (unsigned long)fread((unsigned char *)szFile, 1, 2048, fp);
						if(uRead == 0) break;
						sha256_hash((unsigned char *)szFile, uRead, &sha32);
						if(uRead != 2048) break;
					}
					sha256_end((unsigned char *)aFileKey, &sha32);
				}

				fclose(fp); fp = NULL;

				sha256_begin(&sha32);
				sha256_hash((unsigned char *)paKey2, uKeyLen2, &sha32);
				sha256_end((unsigned char *)aPasswordKey, &sha32);

				mem_erase((unsigned char *)paKey2, uKeyLen2);
				SAFE_DELETE_ARRAY(paKey);

				sha256_begin(&sha32);
				sha256_hash(aPasswordKey, 32, &sha32);
				sha256_hash(aFileKey, 32, &sha32);
				sha256_end((unsigned char *)m_pMasterKey, &sha32);

				mem_erase((unsigned char *)aPasswordKey, 32);
				mem_erase((unsigned char *)aFileKey, 32);
				return PWE_SUCCESS;
			}
			else // pARI is not NULL: save key to disk
			{
				FILE *fp;
				unsigned char aRandomBytes[32];

				fp = _tfopen(szFile, _T("rb")); // Does the file exist already?
				if((fp != NULL) && (bOverwrite == FALSE)) { fclose(fp); fp = NULL; return PWE_NOFILEACCESS_READ; }
				if(fp != NULL) { fclose(fp); fp = NULL; } // We must close it before opening for write

				if(pARI->GenerateRandomSequence(32, aRandomBytes) == FALSE) return PWE_INVALID_RANDOMSOURCE;

				fp = _tfopen(szFile, _T("wb"));
				if(fp == NULL) return PWE_NOFILEACCESS_WRITE;
				if(SaveHexKey32(fp, aRandomBytes) == FALSE) { fclose(fp); fp = NULL; return PWE_FILEERROR_WRITE; }
				fclose(fp); fp = NULL;

				ASSERT(uKeyLen2 != 0);
				sha256_begin(&sha32);
				sha256_hash((unsigned char *)paKey2, uKeyLen2, &sha32);
				sha256_end((unsigned char *)aPasswordKey, &sha32);

				mem_erase((unsigned char *)paKey2, uKeyLen2);
				SAFE_DELETE_ARRAY(paKey);

				sha256_begin(&sha32);
				sha256_hash(aPasswordKey, 32, &sha32);
				sha256_hash(aRandomBytes, 32, &sha32);
				sha256_end((unsigned char *)m_pMasterKey, &sha32);

				mem_erase((unsigned char *)aPasswordKey, 32);
				mem_erase((unsigned char *)aFileKey, 32);
				return PWE_SUCCESS;
			}
		}
		
	}*/

	// return PWE_UNKNOWN; // Unreachable anyway
}

BOOL CPwManager::SetAlgorithm(int nAlgorithm)
{
	ASSERT((nAlgorithm == ALGO_AES) || (nAlgorithm == ALGO_TWOFISH));
	if((nAlgorithm != ALGO_AES) && (nAlgorithm != ALGO_TWOFISH)) return FALSE;

	m_nAlgorithm = nAlgorithm;
	return TRUE;
}

int CPwManager::GetAlgorithm()
{
	return m_nAlgorithm;
}

void CPwManager::_AllocEntries(DWORD uEntries)
{
	PW_ENTRY *p;
	DWORD dwEntries;

	ASSERT((uEntries != 0) && (uEntries != DWORD_MAX));
	if(uEntries == 0) return;

	dwEntries = m_dwNumEntries;

	// If we already have allocated enough entries just return
	if(uEntries <= m_dwMaxEntries) return;

	p = new PW_ENTRY[uEntries];
	if(p == NULL) { ASSERT(FALSE); return; }
	memset(p, 0, sizeof(PW_ENTRY) * uEntries);

	if((m_dwNumEntries > 0) && (m_pEntries != NULL))
	{
		memcpy(p, m_pEntries, sizeof(PW_ENTRY) * m_dwNumEntries);
		_DeleteEntryList(FALSE);
	}

	m_dwNumEntries = dwEntries;
	m_dwMaxEntries = uEntries;
	m_pEntries = p;
}

void CPwManager::_AllocGroups(DWORD uGroups)
{
	PW_GROUP *p;
	DWORD dwGroups;

	ASSERT((uGroups != 0) && (uGroups != DWORD_MAX));
	if((uGroups == 0) || (uGroups == DWORD_MAX)) return;

	dwGroups = m_dwNumGroups;

	// If we already have allocated enough entries just return
	if(uGroups <= m_dwMaxGroups) return;

	p = new PW_GROUP[uGroups];
	if(p == NULL) { ASSERT(FALSE); return; }
	memset(p, 0, sizeof(PW_GROUP) * uGroups);

	if((m_dwNumGroups > 0) && (m_pGroups != NULL))
	{
		memcpy(p, m_pGroups, sizeof(PW_GROUP) * m_dwNumGroups);
		_DeleteGroupList(FALSE);
	}

	m_dwNumGroups = dwGroups;
	m_dwMaxGroups = uGroups;
	m_pGroups = p;
}

void CPwManager::_DeleteEntryList(BOOL bFreeStrings)
{
	unsigned long uCurrentEntry;

	if(m_pEntries == NULL) return; // Nothing to delete

	if(bFreeStrings == TRUE)
	{
		for(uCurrentEntry = 0; uCurrentEntry < m_dwNumEntries; uCurrentEntry++)
		{
			SAFE_DELETE_ARRAY(m_pEntries[uCurrentEntry].pszTitle);
			SAFE_DELETE_ARRAY(m_pEntries[uCurrentEntry].pszURL);
			SAFE_DELETE_ARRAY(m_pEntries[uCurrentEntry].pszUserName);
			SAFE_DELETE_ARRAY(m_pEntries[uCurrentEntry].pszPassword);
			SAFE_DELETE_ARRAY(m_pEntries[uCurrentEntry].pszAdditional);
			SAFE_DELETE_ARRAY(m_pEntries[uCurrentEntry].pszBinaryDesc);
			SAFE_DELETE_ARRAY(m_pEntries[uCurrentEntry].pBinaryData);
		}
	}

	if(m_dwNumEntries != 0) // Erase ALL data
	{
		mem_erase((unsigned char *)m_pEntries,
			sizeof(PW_ENTRY) * m_dwNumEntries);
	}

	SAFE_DELETE_ARRAY(m_pEntries);

	m_dwNumEntries = 0;
	m_dwMaxEntries = 0;
}

void CPwManager::_DeleteGroupList(BOOL bFreeStrings)
{
	unsigned long uCurrentGroup;

	if(m_pGroups == NULL) return; // Nothing to delete

	if(bFreeStrings == TRUE)
	{
		for(uCurrentGroup = 0; uCurrentGroup < m_dwNumGroups; uCurrentGroup++)
		{
			SAFE_DELETE_ARRAY(m_pGroups[uCurrentGroup].pszGroupName);
		}
	}

	if(m_dwNumGroups != 0)
	{
		mem_erase((unsigned char *)m_pGroups,
			sizeof(PW_GROUP) * m_dwNumGroups);
	}

	SAFE_DELETE_ARRAY(m_pGroups);

	m_dwNumGroups = 0;
	m_dwMaxGroups = 0;
}

DWORD CPwManager::GetNumberOfEntries()
{
	return m_dwNumEntries;
}

DWORD CPwManager::GetNumberOfGroups()
{
	return m_dwNumGroups;
}

PW_ENTRY *CPwManager::GetEntry(DWORD dwIndex)
{
	ASSERT(dwIndex < m_dwNumEntries);
	if(dwIndex >= m_dwNumEntries) return NULL;

	return &m_pEntries[dwIndex];
}

PW_ENTRY *CPwManager::GetEntryByGroup(DWORD idGroup, DWORD dwIndex)
{
	DWORD dwPos;

	ASSERT(idGroup != DWORD_MAX);
	if(idGroup == DWORD_MAX) return NULL;
	ASSERT(dwIndex < m_dwNumEntries);
	if(dwIndex >= m_dwNumEntries) return NULL;

	dwPos = GetEntryByGroupN(idGroup, dwIndex);

	if(dwPos != DWORD_MAX) return &m_pEntries[dwPos];

	ASSERT(FALSE);
	return NULL;
}

DWORD CPwManager::GetEntryByGroupN(DWORD idGroup, DWORD dwIndex)
{
	DWORD uCurrentEntry, uEntryInGroupCounter = 0;

	ASSERT(idGroup != DWORD_MAX);
	if(idGroup == DWORD_MAX) return DWORD_MAX;
	ASSERT(dwIndex < m_dwNumEntries);
	if(dwIndex >= m_dwNumEntries) return DWORD_MAX;

	for(uCurrentEntry = 0; uCurrentEntry < m_dwNumEntries; uCurrentEntry++)
	{
		if(idGroup == m_pEntries[uCurrentEntry].uGroupId)
		{
			if(dwIndex == uEntryInGroupCounter) return uCurrentEntry;
			uEntryInGroupCounter++;
		}
	}

	return DWORD_MAX;
}

PW_ENTRY *CPwManager::GetEntryByUuid(BYTE *pUuid)
{
	ASSERT(pUuid != NULL); if(pUuid == NULL) return NULL;

	DWORD dwEntryIndex = GetEntryByUuidN(pUuid);

	ASSERT(dwEntryIndex != DWORD_MAX);
	if(dwEntryIndex == DWORD_MAX) return NULL;

	return &m_pEntries[dwEntryIndex];
}

DWORD CPwManager::GetEntryByUuidN(BYTE *pUuid)
{
	ASSERT(pUuid != NULL); if(pUuid == NULL) return DWORD_MAX;

	for(DWORD dw = 0; dw < m_dwNumEntries; dw++)
	{
		if(memcmp(m_pEntries[dw].uuid, pUuid, 16) == 0) return dw;
	}

	// Don't ASSERT here, just return DWORD_MAX
	return DWORD_MAX;
}

DWORD CPwManager::GetEntryPosInGroup(PW_ENTRY *pEntry)
{
	DWORD uCurrentEntry, pos = 0;

	ASSERT(pEntry != NULL); if(pEntry == NULL) return DWORD_MAX;

	for(uCurrentEntry = 0; uCurrentEntry < m_dwNumEntries; uCurrentEntry++)
	{
		if(m_pEntries[uCurrentEntry].uGroupId == pEntry->uGroupId)
		{
			if(memcmp(m_pEntries[uCurrentEntry].uuid, pEntry->uuid, 16) == 0)
				return pos;

			pos++;
		}
	}

	return DWORD_MAX;
}

PW_ENTRY *CPwManager::GetLastEditedEntry()
{
	return m_pLastEditedEntry;
}

PW_GROUP *CPwManager::GetGroup(DWORD dwIndex)
{
	ASSERT(dwIndex < m_dwNumGroups);
	if(dwIndex >= m_dwNumGroups) return NULL;

	return &m_pGroups[dwIndex];
}

PW_GROUP *CPwManager::GetGroupById(DWORD idGroup)
{
	DWORD dwIndex = GetGroupByIdN(idGroup);
	if(dwIndex == DWORD_MAX) return NULL;
	return &m_pGroups[dwIndex];
}

DWORD CPwManager::GetGroupByIdN(DWORD idGroup)
{
	DWORD uCurrentEntry;

	for(uCurrentEntry = 0; uCurrentEntry < m_dwNumGroups; uCurrentEntry++)
	{
		if(m_pGroups[uCurrentEntry].uGroupId == idGroup) return uCurrentEntry;
	}

	return DWORD_MAX;
}

DWORD CPwManager::GetGroupId(const TCHAR *pszGroupName)
{
	DWORD i;

	ASSERT(pszGroupName != NULL); if(pszGroupName == NULL) return DWORD_MAX;

	for(i = 0; i < m_dwNumGroups; i++)
	{
		if(_tcsicmp(m_pGroups[i].pszGroupName, pszGroupName) == 0)
			return m_pGroups[i].uGroupId;
	}

	return DWORD_MAX;
}

DWORD CPwManager::GetGroupIdByIndex(DWORD uGroupIndex)
{
	if(uGroupIndex >= m_dwNumGroups) return DWORD_MAX;
	return m_pGroups[uGroupIndex].uGroupId;
}

DWORD CPwManager::GetNumberOfItemsInGroup(const TCHAR *pszGroup)
{
	ASSERT(pszGroup != NULL); if(pszGroup == NULL) return 0;

	return GetNumberOfItemsInGroupN(GetGroupId(pszGroup));
}

DWORD CPwManager::GetNumberOfItemsInGroupN(DWORD idGroup)
{
	DWORD i;
	DWORD n;

	ASSERT(idGroup != DWORD_MAX);
	if(idGroup == DWORD_MAX) return 0;

	n = 0;
	for(i = 0; i < m_dwNumEntries; i++)
	{
		if(m_pEntries[i].uGroupId == idGroup) n++;
	}

	return n;
}

BOOL CPwManager::AddEntry(const PW_ENTRY *pTemplate)
{
	PW_ENTRY pT;

	// Don't ASSERT_ENTRY the pTemplate!
	ASSERT(pTemplate != NULL); if(pTemplate == NULL) return FALSE;
	ASSERT((pTemplate->uGroupId != 0) && (pTemplate->uGroupId != DWORD_MAX));
	if((pTemplate->uGroupId == 0) || (pTemplate->uGroupId == DWORD_MAX)) return FALSE;

	// If we don't have enough allocated entries, allocate 32 more
	if(m_dwNumEntries == m_dwMaxEntries)
		_AllocEntries(m_dwMaxEntries + 32);

	pT = *pTemplate; // Copy parameter to local temporary variable

	if(memcmp(pT.uuid, g_uuidZero, 16) == 0) // Shall we create a new UUID?
	{
		////!!! kf !!
		//randCreateUUID(pT.uuid, &m_random); // Create it!
		////ASSERT(GetEntryByUuidN(pT.uuid) == DWORD_MAX);
	}

	// Map NULL pointers to valid pointers to a zero-length string
	if(pT.pszTitle == NULL) pT.pszTitle = (TCHAR *)g_pNullString;
	if(pT.pszUserName == NULL) pT.pszUserName = (TCHAR *)g_pNullString;
	if(pT.pszURL == NULL) pT.pszURL = (TCHAR *)g_pNullString;
	if(pT.pszPassword == NULL) pT.pszPassword = (TCHAR *)g_pNullString;
	if(pT.pszAdditional == NULL) pT.pszAdditional = (TCHAR *)g_pNullString;
	if(pT.pszBinaryDesc == NULL) pT.pszBinaryDesc = (TCHAR *)g_pNullString;

	m_dwNumEntries++;
	return SetEntry(m_dwNumEntries - 1, &pT);
}

BOOL CPwManager::AddGroup(const PW_GROUP *pTemplate)
{
	PW_GROUP pT;
	DWORD i, t = 0, b;

	ASSERT(pTemplate != NULL); if(pTemplate == NULL) return FALSE;

	pT = *pTemplate; // Copy parameter to local temporary variable

	if((pT.uGroupId == 0) || (pT.uGroupId == DWORD_MAX))
	{
		while(1) // Generate a new group ID that doesn't exist already
		{
			b = 0;
			t = randXorShift();
			if((t == 0) || (t == DWORD_MAX)) continue;
			for(i = 0; i < m_dwNumGroups; i++)
			{
				if(m_pGroups[i].uGroupId == t) b = 1;
			}

			if(b == 0) break;
		}
	}
	else t = pT.uGroupId;

	pT.uGroupId = t;

	if(m_dwNumGroups == m_dwMaxGroups)
		_AllocGroups(m_dwMaxGroups + 8);

	m_dwNumGroups++;

	return SetGroup(m_dwNumGroups - 1, &pT);
}

BOOL CPwManager::SetGroup(DWORD dwIndex, PW_GROUP *pTemplate)
{
	DWORD slen;

	ASSERT(dwIndex < m_dwNumGroups);
	ASSERT(pTemplate != NULL);
	ASSERT((pTemplate->uGroupId != 0) && (pTemplate->uGroupId != DWORD_MAX));
	if((pTemplate->uGroupId == 0) || (pTemplate->uGroupId == DWORD_MAX)) return FALSE;

	slen = _tcslen(pTemplate->pszGroupName);
	SAFE_DELETE_ARRAY(m_pGroups[dwIndex].pszGroupName);
	m_pGroups[dwIndex].pszGroupName = new TCHAR[slen + 1];
	_tcscpy(m_pGroups[dwIndex].pszGroupName, pTemplate->pszGroupName);

	m_pGroups[dwIndex].uGroupId = pTemplate->uGroupId;
	m_pGroups[dwIndex].uImageId = pTemplate->uImageId;
	m_pGroups[dwIndex].usLevel = pTemplate->usLevel;
	m_pGroups[dwIndex].dwFlags = pTemplate->dwFlags;

	m_pGroups[dwIndex].tCreation = pTemplate->tCreation;
	m_pGroups[dwIndex].tLastMod = pTemplate->tLastMod;
	m_pGroups[dwIndex].tLastAccess = pTemplate->tLastAccess;
	m_pGroups[dwIndex].tExpire = pTemplate->tExpire;

	return TRUE;
}

BOOL CPwManager::DeleteEntry(DWORD dwIndex)
{
	DWORD i;

	ASSERT(dwIndex < m_dwNumEntries); if(dwIndex >= m_dwNumEntries) return FALSE;

	ASSERT_ENTRY(&m_pEntries[dwIndex]);

	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszTitle);
	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszURL);
	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszUserName);
	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszPassword);
	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszAdditional);
	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszBinaryDesc);
	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pBinaryData);

	if(dwIndex != (m_dwNumEntries - 1))
	{
		for(i = dwIndex; i < (m_dwNumEntries - 1); i++)
		{
			m_pEntries[i] = m_pEntries[i+1];
		}
	}

	mem_erase((unsigned char *)&m_pEntries[m_dwNumEntries - 1], sizeof(PW_ENTRY));
	m_dwNumEntries--;
	return TRUE;
}

BOOL CPwManager::DeleteGroupById(DWORD uGroupId)
{
	DWORD i = 0, inx;
	PW_ENTRY *p;

	ASSERT(GetGroupById(uGroupId) != NULL);

	if(m_dwNumEntries != 0)
	{
		while(1) // Remove all items in that group
		{
			p = &m_pEntries[i];
			if(p->uGroupId == uGroupId)
			{
				/*VERIFY(*/ DeleteEntry(i) ; //);
				i--;
			}

			i++;
			if(i >= m_dwNumEntries) break;
		}
	}

	inx = GetGroupByIdN(uGroupId);
	SAFE_DELETE_ARRAY(m_pGroups[inx].pszGroupName);

	if(inx != (m_dwNumGroups - 1))
	{
		for(i = inx; i < (m_dwNumGroups - 1); i++)
		{
			m_pGroups[i] = m_pGroups[i + 1];
		}
	}

	mem_erase((unsigned char *)&m_pGroups[m_dwNumGroups - 1], sizeof(PW_GROUP));
	m_dwNumGroups--;

	FixGroupTree();
	return TRUE;
}

BOOL CPwManager::SetEntry(DWORD dwIndex, PW_ENTRY *pTemplate)
{
	DWORD slen;

	ASSERT(dwIndex < m_dwNumEntries);
	if(dwIndex >= m_dwNumEntries) return FALSE;

	ASSERT_ENTRY(pTemplate);
	if(pTemplate == NULL) return FALSE;

	ASSERT((pTemplate->uGroupId != 0) && (pTemplate->uGroupId != DWORD_MAX));
	if((pTemplate->uGroupId == 0) || (pTemplate->uGroupId == DWORD_MAX)) return FALSE;

	if(pTemplate->pszTitle == NULL) return FALSE;
	if(pTemplate->pszUserName == NULL) return FALSE;
	if(pTemplate->pszURL == NULL) return FALSE;
	if(pTemplate->pszPassword == NULL) return FALSE;
	if(pTemplate->pszAdditional == NULL) return FALSE;

	memcpy(m_pEntries[dwIndex].uuid, pTemplate->uuid, 16);
	m_pEntries[dwIndex].uGroupId = pTemplate->uGroupId;
	m_pEntries[dwIndex].uImageId = pTemplate->uImageId;

	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszTitle);
	slen = _tcslen(pTemplate->pszTitle);
	m_pEntries[dwIndex].pszTitle = new TCHAR[slen + 1];
	if(slen != 0)
		_tcscpy(m_pEntries[dwIndex].pszTitle, pTemplate->pszTitle);
	else
		m_pEntries[dwIndex].pszTitle[0] = 0;

	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszUserName);
	slen = _tcslen(pTemplate->pszUserName);
	m_pEntries[dwIndex].pszUserName = new TCHAR[slen + 1];
	if(slen != 0)
		_tcscpy(m_pEntries[dwIndex].pszUserName, pTemplate->pszUserName);
	else
		m_pEntries[dwIndex].pszUserName[0] = 0;

	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszURL);
	slen = _tcslen(pTemplate->pszURL);
	m_pEntries[dwIndex].pszURL = new TCHAR[slen + 1];
	if(slen != 0)
		_tcscpy(m_pEntries[dwIndex].pszURL, pTemplate->pszURL);
	else
		m_pEntries[dwIndex].pszURL[0] = 0;

	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszPassword);
	slen = _tcslen(pTemplate->pszPassword);
	m_pEntries[dwIndex].pszPassword = new TCHAR[slen + 1];
	if(slen != 0)
		_tcscpy(m_pEntries[dwIndex].pszPassword, pTemplate->pszPassword);
	else
		m_pEntries[dwIndex].pszPassword[0] = 0;

	SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszAdditional);
	slen = _tcslen(pTemplate->pszAdditional);
	m_pEntries[dwIndex].pszAdditional = new TCHAR[slen + 1];
	if(slen != 0)
		_tcscpy(m_pEntries[dwIndex].pszAdditional, pTemplate->pszAdditional);
	else
		m_pEntries[dwIndex].pszAdditional[0] = 0;

	if(!((m_pEntries[dwIndex].pBinaryData == pTemplate->pBinaryData) && (m_pEntries[dwIndex].pszBinaryDesc == pTemplate->pszBinaryDesc)))
	{
		SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pszBinaryDesc);
		slen = _tcslen(pTemplate->pszBinaryDesc);
		m_pEntries[dwIndex].pszBinaryDesc = new TCHAR[slen + 1];
		if(slen != 0)
			_tcscpy(m_pEntries[dwIndex].pszBinaryDesc, pTemplate->pszBinaryDesc);
		else
			m_pEntries[dwIndex].pszBinaryDesc[0] = 0;

		SAFE_DELETE_ARRAY(m_pEntries[dwIndex].pBinaryData);
		slen = pTemplate->uBinaryDataLen;
		if((pTemplate->pBinaryData != NULL) && (slen != 0))
		{
			m_pEntries[dwIndex].pBinaryData = new BYTE[slen];
			memcpy(m_pEntries[dwIndex].pBinaryData, pTemplate->pBinaryData, slen);
		}
		else
			m_pEntries[dwIndex].pBinaryData = NULL;

		m_pEntries[dwIndex].uBinaryDataLen = pTemplate->uBinaryDataLen;
		if(m_pEntries[dwIndex].pBinaryData == NULL) m_pEntries[dwIndex].uBinaryDataLen = 0;
	}

	m_pEntries[dwIndex].uPasswordLen = _tcslen(m_pEntries[dwIndex].pszPassword);
	LockEntryPassword(&m_pEntries[dwIndex]);

	m_pEntries[dwIndex].tCreation = pTemplate->tCreation;
	m_pEntries[dwIndex].tLastMod = pTemplate->tLastMod;
	m_pEntries[dwIndex].tLastAccess = pTemplate->tLastAccess;
	m_pEntries[dwIndex].tExpire = pTemplate->tExpire;

	ASSERT_ENTRY((&m_pEntries[dwIndex]));
	m_pLastEditedEntry = &m_pEntries[dwIndex];
	return TRUE;
}

void CPwManager::LockEntryPassword(PW_ENTRY *pEntry)
{
	ASSERT_ENTRY(pEntry); if(pEntry == NULL) return;
	ASSERT(pEntry->pszPassword != NULL); if(pEntry->pszPassword == NULL) return;

	if(pEntry->uPasswordLen != 0)
		arcfourCrypt((BYTE *)pEntry->pszPassword, pEntry->uPasswordLen * sizeof(TCHAR),
			m_pSessionKey, PWM_SESSION_KEY_SIZE);
}

void CPwManager::UnlockEntryPassword(PW_ENTRY *pEntry)
{
	ASSERT_ENTRY(pEntry);
	LockEntryPassword(pEntry); // OFB encryption mode
}

void CPwManager::NewDatabase()
{
	_DeleteEntryList(TRUE); // Delete really everything, the strings too
	_DeleteGroupList(TRUE);

	_AllocGroups(PWM_NUM_INITIAL_GROUPS); // Allocate some space for the new items
	_AllocEntries(PWM_NUM_INITIAL_ENTRIES);
}

#define _OPENDB_FAIL_LIGHT \
{ \
	if(pVirtualFile != NULL) \
	{ \
		mem_erase((unsigned char *)pVirtualFile, uAllocated); \
		SAFE_DELETE_ARRAY(pVirtualFile); \
	} \
	m_dwKeyEncRounds = PWM_STD_KEYENCROUNDS; \
}
#define _OPENDB_FAIL \
{ \
	_OPENDB_FAIL_LIGHT; \
	SAFE_DELETE_ARRAY(pwGroupTemplate.pszGroupName); \
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszTitle); \
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszURL); \
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszUserName); \
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszPassword); \
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszAdditional); \
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszBinaryDesc); \
	SAFE_DELETE_ARRAY(pwEntryTemplate.pBinaryData); \
	return PWE_INVALID_FILESTRUCTURE; \
}

#define RESET_TIME_FIELD_NORMAL(pTimeEx) { \
	(pTimeEx)->btDay = 1; (pTimeEx)->btHour = 0; (pTimeEx)->btMinute = 0; \
	(pTimeEx)->btMonth = 1; (pTimeEx)->btSecond = 0; (pTimeEx)->shYear = 2004; }
#define RESET_TIME_FIELD_EXPIRE(pTimeEx) { \
	(pTimeEx)->btDay = 28; (pTimeEx)->btHour = 23; (pTimeEx)->btMinute = 59; \
	(pTimeEx)->btMonth = 12; (pTimeEx)->btSecond = 59; (pTimeEx)->shYear = 4092; }

#define RESET_PWG_TEMPLATE(ptrx) { \
	memset(ptrx, 0, sizeof(PW_GROUP)); \
	RESET_TIME_FIELD_NORMAL(&(ptrx)->tCreation); RESET_TIME_FIELD_NORMAL(&(ptrx)->tLastMod); \
	RESET_TIME_FIELD_NORMAL(&(ptrx)->tLastAccess); RESET_TIME_FIELD_EXPIRE(&(ptrx)->tExpire); }
#define RESET_PWE_TEMPLATE(ptrx) { \
	memset(ptrx, 0, sizeof(PW_ENTRY)); \
	RESET_TIME_FIELD_NORMAL(&(ptrx)->tCreation); RESET_TIME_FIELD_NORMAL(&(ptrx)->tLastMod); \
	RESET_TIME_FIELD_NORMAL(&(ptrx)->tLastAccess); RESET_TIME_FIELD_EXPIRE(&(ptrx)->tExpire); }

// If bIgnoreCorrupted is TRUE the manager will try to ignore all database file
// errors, i.e. try to read as much as possible instead of breaking out at the
// first error.
// To open a file normally, set bIgnoreCorrupted to FALSE (default).
// To open a file in rescue mode, set it to TRUE.
int CPwManager::OpenDatabase(const TCHAR *pszFile, PWDB_REPAIR_INFO *pRepair)
{
	FILE *fp;
	char *pVirtualFile;
	unsigned long uFileSize, uAllocated, uEncryptedPartSize;
	unsigned long pos;
	PW_DBHEADER hdr;
	
    printf("FileName OpenDatabase: %s \n", pszFile) ;
	
	
	sha256_context sha32 ;
	///sha256_ctx sha32;
	
	RD_UINT8 uFinalKey[32];
	char *p;
	char *pStart;
	USHORT usFieldType;
	DWORD dwFieldSize;
	PW_GROUP pwGroupTemplate;
	PW_ENTRY pwEntryTemplate;

	ASSERT(sizeof(char) == 1);

	ASSERT(pszFile != NULL); if(pszFile == NULL) return PWE_INVALID_PARAM;
	ASSERT(pszFile[0] != 0); if(pszFile[0] == 0) return PWE_INVALID_PARAM; // Length != 0

	RESET_PWG_TEMPLATE(&pwGroupTemplate);
	RESET_PWE_TEMPLATE(&pwEntryTemplate);

	// kf !!!
	////if(pRepair != NULL) 
	///	{ ZeroMemory(pRepair, sizeof(PWDB_REPAIR_INFO)); }

	fp = _tfopen(pszFile, _T("rb"));
	if(fp == NULL) return PWE_NOFILEACCESS_READ;

	// Get file size
	fseek(fp, 0, SEEK_END);
	uFileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if(uFileSize < sizeof(PW_DBHEADER))
		{ fclose(fp); return PWE_INVALID_FILEHEADER; }

	// Allocate enough memory to hold the complete file
	uAllocated = uFileSize + 16 + 1 + 8 + 4; // 16 = encryption buffer space, 1+8 = string terminating NULL (UTF-8), 4 unused
	pVirtualFile = new char[uAllocated];
	if(pVirtualFile == NULL) { fclose(fp); return PWE_NO_MEM; }
	memset(&pVirtualFile[uFileSize + 17 - 1], 0, 1 + 8);

	fread(pVirtualFile, 1, uFileSize, fp);
	fclose(fp);

	// Extract header structure from memory file
	memcpy(&hdr, pVirtualFile, sizeof(PW_DBHEADER));

	// Check if we can open this
	if((hdr.dwSignature1 != PWM_DBSIG_1) || (hdr.dwSignature2 != PWM_DBSIG_2))
		{ _OPENDB_FAIL_LIGHT; return PWE_INVALID_FILESIGNATURE; }

	/*
	if((hdr.dwVersion & 0xFFFFFF00) != (PWM_DBVER_DW & 0xFFFFFF00))
	{
		if((hdr.dwVersion == 0x00020000) || (hdr.dwVersion == 0x00020001) || (hdr.dwVersion == 0x00020002))
		{
			if(pVirtualFile != NULL)
			{
				mem_erase((unsigned char *)pVirtualFile, uAllocated);
				SAFE_DELETE_ARRAY(pVirtualFile);
			}
			return (_OpenDatabaseV2(pszFile) != FALSE) ? PWE_SUCCESS : PWE_UNKNOWN;
		}
		else if(hdr.dwVersion <= 0x00010002)
		{
			if(pVirtualFile != NULL)
			{
				mem_erase((unsigned char *)pVirtualFile, uAllocated);
				SAFE_DELETE_ARRAY(pVirtualFile);
			}
			return (_OpenDatabaseV1(pszFile) != FALSE) ? PWE_SUCCESS : PWE_UNKNOWN;
		}
		else { ASSERT(FALSE); _OPENDB_FAIL; }
	}
	*/

	// Select algorithm
	if((hdr.dwFlags & PWM_FLAG_RIJNDAEL) != 0) m_nAlgorithm = ALGO_AES;
	else if((hdr.dwFlags & PWM_FLAG_TWOFISH) != 0) m_nAlgorithm = ALGO_TWOFISH;
	else { ASSERT(FALSE); _OPENDB_FAIL; }

	m_dwKeyEncRounds = hdr.dwKeyEncRounds;

	// Generate m_pTransformedMasterKey from m_pMasterKey
	if(_TransformMasterKey(hdr.aMasterSeed2) == FALSE) { ASSERT(FALSE); _OPENDB_FAIL; }

	// Hash the master password with the salt in the file
	sha256_starts(&sha32);
	sha256_update(&sha32, hdr.aMasterSeed, 16);
	sha256_update(&sha32, m_pTransformedMasterKey, 32);
	sha256_finish(&sha32, (unsigned char *)uFinalKey);

	// kf 
	//sha256_begin(&sha32);
	//sha256_hash(hdr.aMasterSeed, 16, &sha32);
	//sha256_hash(m_pTransformedMasterKey, 32, &sha32);
	//sha256_end((unsigned char *)uFinalKey, &sha32);

	if(pRepair == NULL)
	{
		// ASSERT(((uFileSize - sizeof(PW_DBHEADER)) % 16) == 0);
		if(((uFileSize - sizeof(PW_DBHEADER)) % 16) != 0)
		{
			_OPENDB_FAIL_LIGHT;
			return PWE_INVALID_FILESIZE;
		}
	}
	else // Repair the database
	{
		if(((uFileSize - sizeof(PW_DBHEADER)) % 16) != 0)
		{
			uFileSize -= sizeof(PW_DBHEADER); ASSERT((uFileSize & 0xF) != 0);
			uFileSize &= ~0xF;
			uFileSize += sizeof(PW_DBHEADER);
		}

		ASSERT(((uFileSize - sizeof(PW_DBHEADER)) % 16) == 0);

		pRepair->dwOriginalGroupCount = hdr.dwGroups;
		pRepair->dwOriginalEntryCount = hdr.dwEntries;
	}

	if(m_nAlgorithm == ALGO_AES)
	{
		Rijndael aes;

		// Initialize Rijndael algorithm
		if(aes.init(Rijndael::CBC, Rijndael::Decrypt, uFinalKey,
			Rijndael::Key32Bytes, hdr.aEncryptionIV) != RIJNDAEL_SUCCESS)
			{ _OPENDB_FAIL_LIGHT; return PWE_CRYPT_ERROR; }

		// Decrypt! The first bytes aren't encrypted (that's the header)
		uEncryptedPartSize = (unsigned long)aes.padDecrypt((RD_UINT8 *)pVirtualFile + sizeof(PW_DBHEADER),
			uFileSize - sizeof(PW_DBHEADER), (RD_UINT8 *)pVirtualFile + sizeof(PW_DBHEADER));
	}
	else if(m_nAlgorithm == ALGO_TWOFISH)
	{
		CTwofish twofish;

		if(twofish.init(uFinalKey, 32, hdr.aEncryptionIV) != true)
			{ _OPENDB_FAIL };

		uEncryptedPartSize = (unsigned long)twofish.padDecrypt((RD_UINT8 *)pVirtualFile + sizeof(PW_DBHEADER),
			uFileSize - sizeof(PW_DBHEADER), (RD_UINT8 *)pVirtualFile + sizeof(PW_DBHEADER));
	}
	else
	{
		ASSERT(FALSE); _OPENDB_FAIL; // This should never happen
	}

	// Check for success
	if(pRepair == NULL)
		if((uEncryptedPartSize > 2147483446) || (uEncryptedPartSize == 0))
			{ _OPENDB_FAIL_LIGHT; return PWE_INVALID_KEY; }

	// Check if key is correct (with very high probability)
	if(pRepair == NULL)
	{
		sha256_starts(&sha32);
		sha256_update(&sha32, (unsigned char *)pVirtualFile + sizeof(PW_DBHEADER), uEncryptedPartSize);
		sha256_finish(&sha32, (unsigned char *)uFinalKey);

		// kf 
		//sha256_begin(&sha32);
		//sha256_hash((unsigned char *)pVirtualFile + sizeof(PW_DBHEADER), uEncryptedPartSize, &sha32);
		//sha256_end((unsigned char *)uFinalKey, &sha32);
		
		if(memcmp(hdr.aContentsHash, uFinalKey, 32) != 0)
			{ _OPENDB_FAIL_LIGHT; return PWE_INVALID_KEY; }
	}

	NewDatabase(); // Create a new database and initialize internal structures

	// Add groups from the memory file to the internal structures
	unsigned long uCurGroup;
	BOOL bRet;
	pos = sizeof(PW_DBHEADER);
	pStart = &pVirtualFile[pos];
	for(uCurGroup = 0; uCurGroup < hdr.dwGroups; )
	{
		p = &pVirtualFile[pos];

		/*
		// kf
		if(pRepair != NULL) 
			if(IsBadReadPtr(p, 2) != FALSE) 
				{ _OPENDB_FAIL; }
		*/

		memcpy(&usFieldType, p, 2);
		p += 2; pos += 2;
		if(pos >= uFileSize) { _OPENDB_FAIL; }

		/*
		// kf !!
		if(pRepair != NULL) if(IsBadReadPtr(p, 4) != FALSE) 
			{ _OPENDB_FAIL; }
		*/

		memcpy(&dwFieldSize, p, 4);
		p += 4; pos += 4;
		if(pos >= (uFileSize + dwFieldSize)) 
			{ _OPENDB_FAIL; }

		/**
		// kf !!
		if(pRepair != NULL) if(IsBadReadPtr(p, dwFieldSize) != FALSE) 
			{ _OPENDB_FAIL; }
		*/

		bRet = ReadGroupField(usFieldType, dwFieldSize, (BYTE *)p, &pwGroupTemplate);
		if((usFieldType == 0xFFFF) && (bRet == TRUE))
			uCurGroup++; // Now and ONLY now the counter gets increased

		p += dwFieldSize;
		if(p < pStart) { _OPENDB_FAIL; }
		pos += dwFieldSize;
		if(pos >= uFileSize) { _OPENDB_FAIL; }
	}
	SAFE_DELETE_ARRAY(pwGroupTemplate.pszGroupName);

	// Get the entries
	unsigned long uCurEntry;
	for(uCurEntry = 0; uCurEntry < hdr.dwEntries; )
	{
		p = &pVirtualFile[pos];

		/*
		  // kf 
		  if(pRepair != NULL) if(IsBadReadPtr(p, 2) != FALSE) 
			{ _OPENDB_FAIL; }
		*/

		memcpy(&usFieldType, p, 2);
		p += 2; pos += 2;
		if(pos >= uFileSize) { _OPENDB_FAIL; }

		/**
		// kf
		if(pRepair != NULL) if(IsBadReadPtr(p, 4) != FALSE) 
			{ _OPENDB_FAIL; }
		*/

		memcpy(&dwFieldSize, p, 4);
		p += 4; pos += 4;
		if(pos >= (uFileSize + dwFieldSize)) { _OPENDB_FAIL; }

		/**
		// kf
		if(pRepair != NULL) if(IsBadReadPtr(p, dwFieldSize) != FALSE) 
			{ _OPENDB_FAIL; }
		*/

		bRet = ReadEntryField(usFieldType, dwFieldSize, (BYTE *)p, &pwEntryTemplate);
		if((usFieldType == 0xFFFF) && (bRet == TRUE))
			uCurEntry++; // Now and ONLY now the counter gets increased

		p += dwFieldSize;
		if(p < pStart) { _OPENDB_FAIL; }
		pos += dwFieldSize;
		if(pos >= uFileSize) { _OPENDB_FAIL; }
	}
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszTitle);
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszURL);
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszUserName);
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszPassword);
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszAdditional);
	SAFE_DELETE_ARRAY(pwEntryTemplate.pszBinaryDesc);
	SAFE_DELETE_ARRAY(pwEntryTemplate.pBinaryData);

	// Erase and delete memory file
	mem_erase((unsigned char *)pVirtualFile, uAllocated);
	SAFE_DELETE_ARRAY(pVirtualFile);

	DWORD dwRemovedStreams = _LoadAndRemoveAllMetaStreams();
	if(pRepair != NULL) pRepair->dwRecognizedMetaStreamCount = dwRemovedStreams;
	//VERIFY(DeleteLostEntries() == 0);
	DeleteLostEntries();
	FixGroupTree();

	return PWE_SUCCESS;
}

int CPwManager::SaveDatabase(const TCHAR *pszFile)
{
	/**
	
	 FILE *fp;
	char *pVirtualFile;
	unsigned long uFileSize, uEncryptedPartSize, uAllocated;
	unsigned long i, pos;
	PW_DBHEADER hdr;
	RD_UINT8 uFinalKey[32];
	sha256_ctx sha32;
	USHORT usFieldType;
	DWORD dwFieldSize;
	BYTE aCompressedTime[5];

	ASSERT(pszFile != NULL);
	if(pszFile == NULL) return PWE_INVALID_PARAM;
	ASSERT(_tcslen(pszFile) != 0);
	if(_tcslen(pszFile) == 0) return PWE_INVALID_PARAM;

	_AddAllMetaStreams();

	uFileSize = sizeof(PW_DBHEADER);

	BYTE *pbt;

	ASSERT(sizeof(TCHAR *) == sizeof(DWORD));
	// Get the size of all groups
	for(i = 0; i < m_dwNumGroups; i++)
	{
		uFileSize += 94; // 6+4+6+6+5+6+5+6+5+6+5+6+4+6+6+2+6+4 = 94

		pbt = _StringToUTF8(m_pGroups[i].pszGroupName);
		uFileSize += szlen((char *)pbt) + 1;
		SAFE_DELETE_ARRAY(pbt);
	}

	// Get the size of all entries together
	for(i = 0; i < m_dwNumEntries; i++)
	{
		ASSERT_ENTRY(&m_pEntries[i]);

		UnlockEntryPassword(&m_pEntries[i]);

		uFileSize += 134; // 6+16+6+4+6+4+6+6+6+6+6+6+5+6+5+6+5+6+5+6 = 122

		pbt = _StringToUTF8(m_pEntries[i].pszTitle);
		uFileSize += szlen((char *)pbt) + 1;
		SAFE_DELETE_ARRAY(pbt);

		pbt = _StringToUTF8(m_pEntries[i].pszUserName);
		uFileSize += szlen((char *)pbt) + 1;
		SAFE_DELETE_ARRAY(pbt);

		pbt = _StringToUTF8(m_pEntries[i].pszURL);
		uFileSize += szlen((char *)pbt) + 1;
		SAFE_DELETE_ARRAY(pbt);

		pbt = _StringToUTF8(m_pEntries[i].pszPassword);
		uFileSize += szlen((char *)pbt) + 1;
		SAFE_DELETE_ARRAY(pbt);

		pbt = _StringToUTF8(m_pEntries[i].pszAdditional);
		uFileSize += szlen((char *)pbt) + 1;
		SAFE_DELETE_ARRAY(pbt);

		pbt = _StringToUTF8(m_pEntries[i].pszBinaryDesc);
		uFileSize += szlen((char *)pbt) + 1;
		SAFE_DELETE_ARRAY(pbt);

		uFileSize += m_pEntries[i].uBinaryDataLen;

		LockEntryPassword(&m_pEntries[i]);
	}

	// Round up filesize to 16-byte boundary for Rijndael/Twofish
	uFileSize = (uFileSize + 16) - (uFileSize % 16);

	// Allocate enough memory
	uAllocated = uFileSize + 16;
	pVirtualFile = new char[uAllocated];
	ASSERT(pVirtualFile != NULL);
	if(pVirtualFile == NULL) { _LoadAndRemoveAllMetaStreams(); return PWE_NO_MEM; }

	// Build header structure
	hdr.dwSignature1 = PWM_DBSIG_1;
	hdr.dwSignature2 = PWM_DBSIG_2;

	hdr.dwFlags = PWM_FLAG_SHA2; // The one and only hash algorithm available currently

	if(m_nAlgorithm == ALGO_AES) hdr.dwFlags |= PWM_FLAG_RIJNDAEL;
	else if(m_nAlgorithm == ALGO_TWOFISH) hdr.dwFlags |= PWM_FLAG_TWOFISH;
	else { ASSERT(FALSE); _LoadAndRemoveAllMetaStreams(); return PWE_INVALID_PARAM; }

	hdr.dwVersion = PWM_DBVER_DW;
	hdr.dwGroups = m_dwNumGroups;
	hdr.dwEntries = m_dwNumEntries;
	hdr.dwKeyEncRounds = m_dwKeyEncRounds;

	// Make up the master key hash seed and the encryption IV
	m_random.GetRandomBuffer(hdr.aMasterSeed, 16);
	m_random.GetRandomBuffer((BYTE *)hdr.aEncryptionIV, 16);
	m_random.GetRandomBuffer(hdr.aMasterSeed2, 32);

	// Skip the header, it will be written later
	pos = sizeof(PW_DBHEADER);

	BYTE *pb;

	// Store all groups to memory file
	for(i = 0; i < m_dwNumGroups; i++)
	{
		usFieldType = 0x0001; dwFieldSize = 4;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		memcpy(&pVirtualFile[pos], &m_pGroups[i].uGroupId, 4); pos += 4;

		pb = _StringToUTF8(m_pGroups[i].pszGroupName);
		usFieldType = 0x0002; dwFieldSize = szlen((char *)pb) + 1;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		ASSERT((pb != NULL) && (szlen((char *)pb) == (dwFieldSize - 1)) && ((pos + dwFieldSize) <= uAllocated));
		szcpy(&pVirtualFile[pos], (char *)pb); pos += dwFieldSize;
		SAFE_DELETE_ARRAY(pb);

		usFieldType = 0x0003; dwFieldSize = 5;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		_PwTimeToTime(&m_pGroups[i].tCreation, aCompressedTime);
		memcpy(&pVirtualFile[pos], aCompressedTime, 5); pos += 5;

		usFieldType = 0x0004; dwFieldSize = 5;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		_PwTimeToTime(&m_pGroups[i].tLastMod, aCompressedTime);
		memcpy(&pVirtualFile[pos], aCompressedTime, 5); pos += 5;

		usFieldType = 0x0005; dwFieldSize = 5;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		_PwTimeToTime(&m_pGroups[i].tLastAccess, aCompressedTime);
		memcpy(&pVirtualFile[pos], aCompressedTime, 5); pos += 5;

		usFieldType = 0x0006; dwFieldSize = 5;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		_PwTimeToTime(&m_pGroups[i].tExpire, aCompressedTime);
		memcpy(&pVirtualFile[pos], aCompressedTime, 5); pos += 5;

		usFieldType = 0x0007; dwFieldSize = 4;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		memcpy(&pVirtualFile[pos], &m_pGroups[i].uImageId, 4); pos += 4;

		usFieldType = 0x0008; dwFieldSize = 2;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		memcpy(&pVirtualFile[pos], &m_pGroups[i].usLevel, 2); pos += 2;

		usFieldType = 0x0009; dwFieldSize = 4;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		memcpy(&pVirtualFile[pos], &m_pGroups[i].dwFlags, 4); pos += 4;

		usFieldType = 0xFFFF; dwFieldSize = 0;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
	}

	// Store all entries to memory file
	for(i = 0; i < m_dwNumEntries; i++)
	{
		UnlockEntryPassword(&m_pEntries[i]);

		usFieldType = 0x0001; dwFieldSize = 16;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		memcpy(&pVirtualFile[pos], &m_pEntries[i].uuid, 16); pos += 16;

		usFieldType = 0x0002; dwFieldSize = 4;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		memcpy(&pVirtualFile[pos], &m_pEntries[i].uGroupId, 4); pos += 4;

		usFieldType = 0x0003; dwFieldSize = 4;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		memcpy(&pVirtualFile[pos], &m_pEntries[i].uImageId, 4); pos += 4;

		pb = _StringToUTF8(m_pEntries[i].pszTitle);
		usFieldType = 0x0004;
		dwFieldSize = szlen((char *)pb) + 1; // Add terminating NULL character space
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		ASSERT((pb != NULL) && (szlen((char *)pb) == (dwFieldSize - 1)) && ((pos + dwFieldSize) <= uAllocated));
		szcpy(&pVirtualFile[pos], (char *)pb); pos += dwFieldSize;
		SAFE_DELETE_ARRAY(pb);

		pb = _StringToUTF8(m_pEntries[i].pszURL);
		usFieldType = 0x0005;
		dwFieldSize = szlen((char *)pb) + 1; // Add terminating NULL character space
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		ASSERT((pb != NULL) && (szlen((char *)pb) == (dwFieldSize - 1)) && ((pos + dwFieldSize) <= uAllocated));
		szcpy(&pVirtualFile[pos], (char *)pb); pos += dwFieldSize;
		SAFE_DELETE_ARRAY(pb);

		pb = _StringToUTF8(m_pEntries[i].pszUserName);
		usFieldType = 0x0006;
		dwFieldSize = szlen((char *)pb) + 1; // Add terminating NULL character space
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		ASSERT((pb != NULL) && (szlen((char *)pb) == (dwFieldSize - 1)) && ((pos + dwFieldSize) <= uAllocated));
		szcpy(&pVirtualFile[pos], (char *)pb); pos += dwFieldSize;
		SAFE_DELETE_ARRAY(pb);

		pb = _StringToUTF8(m_pEntries[i].pszPassword);
		usFieldType = 0x0007;
		dwFieldSize = szlen((char *)pb) + 1; // Add terminating NULL character space
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		ASSERT((pb != NULL) && (szlen((char *)pb) == (dwFieldSize - 1)) && ((pos + dwFieldSize) <= uAllocated));
		szcpy(&pVirtualFile[pos], (char *)pb); pos += dwFieldSize;
		if(pb != NULL) mem_erase((unsigned char *)pb, szlen((char *)pb));
		SAFE_DELETE_ARRAY(pb);

		pb = _StringToUTF8(m_pEntries[i].pszAdditional);
		usFieldType = 0x0008;
		dwFieldSize = szlen((char *)pb) + 1; // Add terminating NULL character space
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		ASSERT((pb != NULL) && (szlen((char *)pb) == (dwFieldSize - 1)) && ((pos + dwFieldSize) <= uAllocated));
		szcpy(&pVirtualFile[pos], (char *)pb); pos += dwFieldSize;
		SAFE_DELETE_ARRAY(pb);

		usFieldType = 0x0009; dwFieldSize = 5;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		_PwTimeToTime(&m_pEntries[i].tCreation, aCompressedTime);
		memcpy(&pVirtualFile[pos], aCompressedTime, 5); pos += 5;

		usFieldType = 0x000A; dwFieldSize = 5;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		_PwTimeToTime(&m_pEntries[i].tLastMod, aCompressedTime);
		memcpy(&pVirtualFile[pos], aCompressedTime, 5); pos += 5;

		usFieldType = 0x000B; dwFieldSize = 5;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		_PwTimeToTime(&m_pEntries[i].tLastAccess, aCompressedTime);
		memcpy(&pVirtualFile[pos], aCompressedTime, 5); pos += 5;

		usFieldType = 0x000C; dwFieldSize = 5;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		_PwTimeToTime(&m_pEntries[i].tExpire, aCompressedTime);
		memcpy(&pVirtualFile[pos], aCompressedTime, 5); pos += 5;

		pb = _StringToUTF8(m_pEntries[i].pszBinaryDesc);
		usFieldType = 0x000D;
		dwFieldSize = szlen((char *)pb) + 1;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		ASSERT((pb != NULL) && (szlen((char *)pb) == (dwFieldSize - 1)) && ((pos + dwFieldSize) <= uAllocated));
		szcpy(&pVirtualFile[pos], (char *)pb); pos += dwFieldSize;
		SAFE_DELETE_ARRAY(pb);

		usFieldType = 0x000E; dwFieldSize = m_pEntries[i].uBinaryDataLen;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;
		if((m_pEntries[i].pBinaryData != NULL) && (dwFieldSize != 0))
			memcpy(&pVirtualFile[pos], m_pEntries[i].pBinaryData, dwFieldSize);
		pos += dwFieldSize;

		usFieldType = 0xFFFF; dwFieldSize = 0;
		memcpy(&pVirtualFile[pos], &usFieldType, 2); pos += 2;
		memcpy(&pVirtualFile[pos], &dwFieldSize, 4); pos += 4;

		LockEntryPassword(&m_pEntries[i]);
	}
	ASSERT((pos <= uFileSize) && ((pos + 33) > uAllocated));

	sha256_begin(&sha32);
	sha256_hash((unsigned char *)pVirtualFile + sizeof(PW_DBHEADER), pos - sizeof(PW_DBHEADER), &sha32);
	sha256_end((unsigned char *)hdr.aContentsHash, &sha32);

	// Now we have all to build up the header
	memcpy(pVirtualFile, &hdr, sizeof(PW_DBHEADER));

	// Generate m_pTransformedMasterKey from m_pMasterKey
	if(_TransformMasterKey(hdr.aMasterSeed2) == FALSE)
		{ ASSERT(FALSE); SAFE_DELETE_ARRAY(pVirtualFile); _LoadAndRemoveAllMetaStreams(); return PWE_CRYPT_ERROR; }

	// Hash the master password with the generated hash salt
	sha256_begin(&sha32);
	sha256_hash(hdr.aMasterSeed, 16, &sha32);
	sha256_hash(m_pTransformedMasterKey, 32, &sha32);
	sha256_end((unsigned char *)uFinalKey, &sha32);

	if(m_nAlgorithm == ALGO_AES)
	{
		Rijndael aes;

		// Initialize Rijndael/AES
		if(aes.init(Rijndael::CBC, Rijndael::Encrypt, uFinalKey,
			Rijndael::Key32Bytes, hdr.aEncryptionIV) != RIJNDAEL_SUCCESS)
		{
			SAFE_DELETE_ARRAY(pVirtualFile);
			return PWE_CRYPT_ERROR;
		}

		uEncryptedPartSize = (unsigned long)aes.padEncrypt((RD_UINT8 *)pVirtualFile +
			sizeof(PW_DBHEADER), pos - sizeof(PW_DBHEADER), (RD_UINT8 *)pVirtualFile +
			sizeof(PW_DBHEADER));
	}
	else if(m_nAlgorithm == ALGO_TWOFISH)
	{
		CTwofish twofish;

		if(twofish.init(uFinalKey, 32, hdr.aEncryptionIV) == false)
		{
			SAFE_DELETE_ARRAY(pVirtualFile);
			_LoadAndRemoveAllMetaStreams();
			return PWE_CRYPT_ERROR;
		}

		uEncryptedPartSize = (unsigned long)twofish.padEncrypt(
			(RD_UINT8 *)pVirtualFile + sizeof(PW_DBHEADER),
			pos - sizeof(PW_DBHEADER), (RD_UINT8 *)pVirtualFile + sizeof(PW_DBHEADER));
	}
	else
	{
		ASSERT(FALSE); _OPENDB_FAIL_LIGHT; return PWE_INVALID_PARAM;
	}

	// Check if all went correct
	ASSERT((uEncryptedPartSize % 16) == 0);
	if((uEncryptedPartSize > 2147483446) || (uEncryptedPartSize == 0))
	{
		ASSERT(FALSE); SAFE_DELETE_ARRAY(pVirtualFile); _LoadAndRemoveAllMetaStreams();
		return PWE_CRYPT_ERROR;
	}

	fp = _tfopen(pszFile, _T("wb"));
	if(fp == NULL)
	{
		mem_erase((unsigned char *)pVirtualFile, uAllocated);
		SAFE_DELETE_ARRAY(pVirtualFile); _LoadAndRemoveAllMetaStreams();
		return PWE_NOFILEACCESS_WRITE;
	}

	// Write memory file to disk
	if(fwrite(pVirtualFile, 1, uEncryptedPartSize + sizeof(PW_DBHEADER), fp) !=
		uEncryptedPartSize + sizeof(PW_DBHEADER))
	{
		mem_erase((unsigned char *)pVirtualFile, uAllocated);
		SAFE_DELETE_ARRAY(pVirtualFile); _LoadAndRemoveAllMetaStreams();
		return PWE_FILEERROR_WRITE;
	}

	// Close file, erase and delete memory
	fclose(fp); fp = NULL;

	mem_erase((unsigned char *)pVirtualFile, uAllocated);
	SAFE_DELETE_ARRAY(pVirtualFile);
	_LoadAndRemoveAllMetaStreams();
	*/

	return PWE_SUCCESS;
}

DWORD CPwManager::Find(const TCHAR *pszFindString, BOOL bCaseSensitive, DWORD fieldFlags, DWORD nStart)
{
	/**
	// kf
	DWORD i;
	CString strFind;
	CString strEntry;

	if(nStart >= (int)m_dwNumEntries) return DWORD_MAX;
	ASSERT(pszFindString != NULL); if(pszFindString == NULL) return DWORD_MAX;

	strFind = pszFindString;
	if(bCaseSensitive == FALSE) strFind.MakeLower();

	if((strFind.GetLength() == 0) || (strFind == _T("*"))) return nStart;

	for(i = nStart; i < (int)m_dwNumEntries; i++)
	{
		if(fieldFlags & PWMF_TITLE)
		{
			strEntry = m_pEntries[i].pszTitle;
			if(bCaseSensitive == FALSE) strEntry.MakeLower();

			if(strEntry.Find(strFind) != -1) return i;
		}
		if(fieldFlags & PWMF_USER)
		{
			strEntry = m_pEntries[i].pszUserName;
			if(bCaseSensitive == FALSE) strEntry.MakeLower();

			if(strEntry.Find(strFind) != -1) return i;
		}
		if(fieldFlags & PWMF_URL)
		{
			strEntry = m_pEntries[i].pszURL;
			if(bCaseSensitive == FALSE) strEntry.MakeLower();

			if(strEntry.Find(strFind) != -1) return i;
		}
		if(fieldFlags & PWMF_PASSWORD)
		{
			UnlockEntryPassword(&m_pEntries[i]);
			strEntry = m_pEntries[i].pszPassword;
			LockEntryPassword(&m_pEntries[i]);
			if(bCaseSensitive == FALSE) strEntry.MakeLower();

			if(strEntry.Find(strFind) != -1) return i;
		}
		if(fieldFlags & PWMF_ADDITIONAL)
		{
			strEntry = m_pEntries[i].pszAdditional;
			if(bCaseSensitive == FALSE) strEntry.MakeLower();

			if(strEntry.Find(strFind) != -1) return i;
		}
		if(fieldFlags & PWMF_GROUPNAME)
		{
			DWORD dwGroupIndex = GetGroupByIdN(m_pEntries[i].uGroupId);
			ASSERT(dwGroupIndex != DWORD_MAX);
			strEntry = GetGroup(dwGroupIndex)->pszGroupName;
			if(bCaseSensitive == FALSE) strEntry.MakeLower();

			if(strEntry.Find(strFind) != -1) return i;
		}
	}
	*/

	return DWORD_MAX;
}

void CPwManager::MoveInternal(DWORD nFrom, DWORD nTo)
{
	LONG i;
	LONG dir;
	PW_ENTRY pe;

	if(nFrom == nTo) return;
	if(nFrom >= m_dwNumEntries) return;
	if(nTo >= m_dwNumEntries) return;

	// Set moving direction
	if(nFrom < nTo) dir = 1;
	else dir = -1;

	i = (LONG)nFrom;
	while(1)
	{
		if(i == (LONG)nTo) break;

		pe = m_pEntries[i];
		m_pEntries[i] = m_pEntries[i+dir];
		m_pEntries[i+dir] = pe;

		i += dir;
	}
}

BOOL CPwManager::MoveGroup(DWORD nFrom, DWORD nTo)
{
	LONG i;
	LONG dir;
	PW_GROUP pg;

	ASSERT((nFrom != DWORD_MAX) && (nTo != DWORD_MAX));

	if(nFrom == nTo) TRUE;
	if(nFrom >= m_dwNumGroups) return FALSE;
	if(nTo >= m_dwNumGroups) return FALSE;

	// Set moving direction
	if(nFrom < nTo) dir = 1;
	else dir = -1;

	i = (LONG)nFrom;

	while(1)
	{
		if(i == (LONG)nTo) break;

		pg = m_pGroups[i];
		m_pGroups[i] = m_pGroups[i+dir];
		m_pGroups[i+dir] = pg;

		i += dir;
	}

	FixGroupTree();

	return TRUE;
}

void CPwManager::MoveInGroup(DWORD idGroup, DWORD nFrom, DWORD nTo)
{
	if((nFrom >= m_dwNumEntries) || (nFrom == DWORD_MAX)) return;
	if((nTo >= m_dwNumEntries) || (nTo == DWORD_MAX)) return;
	if(nFrom == nTo) return;

	DWORD dwFromEx = GetEntryByGroupN(idGroup, nFrom);
	DWORD dwToEx = GetEntryByGroupN(idGroup, nTo);

	if((dwFromEx == DWORD_MAX) || (dwToEx == DWORD_MAX)) return;

	MoveInternal(dwFromEx, dwToEx);
}

BOOL CPwManager::GetGroupTree(DWORD idGroup, DWORD *pGroupIndexes)
{
	DWORD i, dwGroupPos;
	USHORT usLevel;

	ASSERT(pGroupIndexes != NULL); if(pGroupIndexes == NULL) return FALSE;

	dwGroupPos = GetGroupByIdN(idGroup);
	ASSERT(dwGroupPos != DWORD_MAX); if(dwGroupPos == DWORD_MAX) return FALSE;

	i = dwGroupPos;
	usLevel = (USHORT)(m_pGroups[i].usLevel + 1);
	while(1)
	{
		if(m_pGroups[i].usLevel == (usLevel - 1))
		{
			usLevel--;
			pGroupIndexes[usLevel] = i;
			if(usLevel == 0) break;
		}

		if(i == 0) { ASSERT(FALSE); return FALSE; }
		i--;
	}

	return TRUE;
}

/*void CPwManager::SortGroupList()
{
	DWORD i, j;
	BOOL bSwapped = TRUE;
	PPW_GROUP p = NULL;
	LPTSTR *pList = NULL;
	LPTSTR *pParents = NULL;
	LPTSTR lpTemp = NULL;
	LPTSTR lp = NULL;
	DWORD dwMaxString = 0;
	USHORT usMaxLevel = 0;
	TCHAR tszTemp[16];
	DWORD dw;
	PW_GROUP pwt;

	if(m_dwNumGroups <= 1) return; // Nothing to sort

	// Search list for longest group name and maximum level
	for(i = 0; i < m_dwNumGroups; i++)
	{
		j = (DWORD)_tcslen(m_pGroups[i].pszGroupName);
		if(j > dwMaxString) dwMaxString = j;

		if(m_pGroups[i].usLevel > usMaxLevel) usMaxLevel = m_pGroups[i].usLevel;
	}

	// Allocate pointer list to parent items
	pParents = new LPTSTR[usMaxLevel + 2];
	ASSERT(pParents != NULL); if(pParents == NULL) return;

	// Calculate maximum length of the biggest FQGN (fully qualified group name)
	// (numchars(base10(0xFFFFFFFF)) = 10) + char(NULL) + char(/n) + char(/n) = 13
	dwMaxString = (dwMaxString + 1) * (usMaxLevel + 1) + 13;

	pList = new LPTSTR[m_dwNumGroups]; // Allocate FQGN list (array of pointers)
	ASSERT(pList != NULL);
	lpTemp = new TCHAR[dwMaxString];
	ASSERT(lpTemp != NULL);

	// Build FQGN list
	for(i = 0; i < m_dwNumGroups; i++)
	{
		pList[i] = new TCHAR[dwMaxString];
		ASSERT(pList[i] != NULL);
		if(pList[i] == NULL)
		{
			for(j = 0; j < i; j++) SAFE_DELETE_ARRAY(pList[j]);
			SAFE_DELETE_ARRAY(pList); SAFE_DELETE_ARRAY(pParents); SAFE_DELETE_ARRAY(lpTemp);
			return; // Failed, too few memory available
		}
		*pList[i] = 0;

		p = &m_pGroups[i]; ASSERT(p != NULL);
		if(p->usLevel != 0)
		{
			for(j = 0; j < p->usLevel; j++)
			{
				_tcscat(pList[i], pParents[j]);
				_tcscat(pList[i], _T("\n"));
			}
		}
		pParents[p->usLevel] = p->pszGroupName;
		_tcscat(pList[i], p->pszGroupName);
		_tcscat(pList[i], _T("\n\n"));

		_ltot((long)p->uGroupId, tszTemp, 10);
		_tcscat(pList[i], tszTemp);

		ASSERT(_tcslen(pList[i]) < dwMaxString);
	}

	while(bSwapped == TRUE) // Stable bubble-sort on the group list
	{
		bSwapped = FALSE;

		for(i = 0; i < (m_dwNumGroups - 1); i++)
		{
			if(_tcsicmp(pList[i], pList[i+1]) > 0)
			{
				_tcscpy(lpTemp, pList[i]); // Swap strings
				_tcscpy(pList[i], pList[i+1]);
				_tcscpy(pList[i+1], lpTemp);

				bSwapped = TRUE;
			}
		}
	}

	for(i = 0; i < (m_dwNumGroups - 1); i++)
	{
		lp = &pList[i][_tcslen(pList[i]) - 1];
		while(1)
		{
			if(*lp == _T('\n')) break;
			lp--;
		}
		lp++;

		dw = (DWORD)_ttol(lp);
		ASSERT(GetGroupById(dw) != NULL);

		for(j = i; j < m_dwNumGroups; j++)
		{
			if(m_pGroups[j].uGroupId == dw) break;
		}

		pwt = m_pGroups[i];
		m_pGroups[i] = m_pGroups[j];
		m_pGroups[j] = pwt;
	}

	for(i = 0; i < m_dwNumGroups; i++) SAFE_DELETE_ARRAY(pList[i]);
	SAFE_DELETE_ARRAY(pList); SAFE_DELETE_ARRAY(pParents); SAFE_DELETE_ARRAY(lpTemp);

	FixGroupTree();
}
*/

/*
void CPwManager::SortGroup(DWORD idGroup, DWORD dwSortByField)
{
	PPW_ENTRY *p;
	PW_ENTRY v;
	DWORD i, j, n = 0, min, t;

	if(m_dwNumEntries <= 1) return; // Nothing to sort

	p = new PPW_ENTRY[m_dwNumEntries];
	if(p == NULL) return;

	// Build pointer array that contains pointers to the elements to sort
	for(i = 0; i < m_dwNumEntries; i++)
	{
		if(m_pEntries[i].uGroupId == idGroup)
		{
			p[n] = &m_pEntries[i];
			n++;
		}
	}
	if(n <= 1) { SAFE_DELETE_ARRAY(p); return; } // Something to sort?

	// Sort the array, using a simple selection sort
	for(i = 0; i < (n - 1); i++)
	{
		min = i;

		for(j = i + 1; j < n; j++)
		{
			switch(dwSortByField)
			{
			case 0:
				if(_tcsicmp(p[j]->pszTitle, p[min]->pszTitle) < 0)
					min = j;
				break;
			case 1:
				if(_tcsicmp(p[j]->pszUserName, p[min]->pszUserName) < 0)
					min = j;
				break;
			case 2:
				if(_tcsicmp(p[j]->pszURL, p[min]->pszURL) < 0)
					min = j;
				break;
			case 3:
				t = min;
				UnlockEntryPassword(p[j]); UnlockEntryPassword(p[t]);
				if(_tcsicmp(p[j]->pszPassword, p[min]->pszPassword) < 0)
					min = j;
				LockEntryPassword(p[j]); LockEntryPassword(p[t]);
				break;
			case 4:
				if(_tcsicmp(p[j]->pszAdditional, p[min]->pszAdditional) < 0)
					min = j;
				break;
			case 5:
				if(_pwtimecmp(&p[j]->tCreation, &p[min]->tCreation) > 0)
					min = j;
				break;
			case 6:
				if(_pwtimecmp(&p[j]->tLastMod, &p[min]->tLastMod) > 0)
					min = j;
				break;
			case 7:
				if(_pwtimecmp(&p[j]->tLastAccess, &p[min]->tLastAccess) > 0)
					min = j;
				break;
			case 8:
				if(_pwtimecmp(&p[j]->tExpire, &p[min]->tExpire) > 0)
					min = j;
				break;
			case 9:
				// Cannot sort by UUID
				break;
			default:
				ASSERT(FALSE);
				if(_tcsicmp(p[j]->pszTitle, p[min]->pszTitle) < 0)
					min = j;
				break;
			}
		}

		v = *p[min];
		*p[min] = *p[i];
		*p[i] = v;
	}

	SAFE_DELETE_ARRAY(p);
}
*/

void CPwManager::_TimeToPwTime(BYTE *pCompressedTime, PW_TIME *pPwTime)
{
	DWORD dwYear, dwMonth, dwDay, dwHour, dwMinute, dwSecond;

	ASSERT((pCompressedTime != NULL) && (pPwTime != NULL));
	if(pPwTime == NULL) return;

	_UnpackStructToTime(pCompressedTime, &dwYear, &dwMonth, &dwDay, &dwHour, &dwMinute, &dwSecond);
	pPwTime->shYear = (USHORT)dwYear;
	pPwTime->btMonth = (BYTE)dwMonth;
	pPwTime->btDay = (BYTE)dwDay;
	pPwTime->btHour = (BYTE)dwHour;
	pPwTime->btMinute = (BYTE)dwMinute;
	pPwTime->btSecond = (BYTE)dwSecond;
}

void CPwManager::_PwTimeToTime(PW_TIME *pPwTime, BYTE *pCompressedTime)
{
	ASSERT((pPwTime != NULL) && (pCompressedTime != NULL));
	if(pPwTime == NULL) return;

	_PackTimeToStruct(pCompressedTime, (DWORD)pPwTime->shYear, (DWORD)pPwTime->btMonth,
		(DWORD)pPwTime->btDay, (DWORD)pPwTime->btHour, (DWORD)pPwTime->btMinute,
		(DWORD)pPwTime->btSecond);
}

BOOL CPwManager::ReadGroupField(USHORT usFieldType, DWORD dwFieldSize, BYTE *pData, PW_GROUP *pGroup)
{
	BYTE aCompressedTime[5];

	switch(usFieldType)
	{
	case 0x0000:
		// Ignore field
		break;
	case 0x0001:
		memcpy(&pGroup->uGroupId, pData, 4);
		break;
	case 0x0002:
		ASSERT(dwFieldSize != 0);
		SAFE_DELETE_ARRAY(pGroup->pszGroupName);
		//pGroup->pszGroupName = _UTF8ToString((UTF8_BYTE *)pData);
		
		pGroup->pszGroupName = new char[strlen((char*)pData)+2] ;
		strcpy(pGroup->pszGroupName, (char*) pData) ;
		
		break;
	case 0x0003:
		memcpy(aCompressedTime, pData, 5);
		_TimeToPwTime(aCompressedTime, &pGroup->tCreation);
		break;
	case 0x0004:
		memcpy(aCompressedTime, pData, 5);
		_TimeToPwTime(aCompressedTime, &pGroup->tLastMod);
		break;
	case 0x0005:
		memcpy(aCompressedTime, pData, 5);
		_TimeToPwTime(aCompressedTime, &pGroup->tLastAccess);
		break;
	case 0x0006:
		memcpy(aCompressedTime, pData, 5);
		_TimeToPwTime(aCompressedTime, &pGroup->tExpire);
		break;
	case 0x0007:
		memcpy(&pGroup->uImageId, pData, 4);
		break;
	case 0x0008:
		memcpy(&pGroup->usLevel, pData, 2);
		break;
	case 0x0009:
		memcpy(&pGroup->dwFlags, pData, 4);
		break;
	case 0xFFFF:
		AddGroup(pGroup);
		SAFE_DELETE_ARRAY(pGroup->pszGroupName);
		RESET_PWG_TEMPLATE(pGroup);
		break;
	default:
		return FALSE; // Field unsupported
	}

	return TRUE; // Field supported
}

BOOL CPwManager::ReadEntryField(USHORT usFieldType, DWORD dwFieldSize, BYTE *pData, PW_ENTRY *pEntry)
{
	BYTE aCompressedTime[5];

	switch(usFieldType)
	{
	case 0x0000:
		// Ignore field
		break;
	case 0x0001:
		memcpy(pEntry->uuid, pData, 16);
		break;
	case 0x0002:
		memcpy(&pEntry->uGroupId, pData, 4);
		break;
	case 0x0003:
		memcpy(&pEntry->uImageId, pData, 4);
		break;
	case 0x0004:
		ASSERT(dwFieldSize != 0);
		SAFE_DELETE_ARRAY(pEntry->pszTitle);
		//pEntry->pszTitle = _UTF8ToString((UTF8_BYTE *)pData);
		
		pEntry->pszTitle = new char[strlen((char*)pData)+2] ;
		strcpy(pEntry->pszTitle, (char*) pData) ;

		break;
	case 0x0005:
		ASSERT(dwFieldSize != 0);
		SAFE_DELETE_ARRAY(pEntry->pszURL);
		//pEntry->pszURL = _UTF8ToString((UTF8_BYTE *)pData);
		pEntry->pszURL = new char[strlen((char*)pData)+2] ;
		strcpy(pEntry->pszURL, (char*) pData) ;
		
		break;
	case 0x0006:
		ASSERT(dwFieldSize != 0);
		SAFE_DELETE_ARRAY(pEntry->pszUserName);
		//pEntry->pszUserName = _UTF8ToString((UTF8_BYTE *)pData);
		pEntry->pszUserName = new char[strlen((char*)pData)+2] ;
		strcpy(pEntry->pszUserName, (char*) pData) ;
		
		break;
	case 0x0007:
		ASSERT(dwFieldSize != 0);
		if(pEntry->pszPassword != NULL)
			mem_erase((unsigned char *)pEntry->pszPassword, _tcslen(pEntry->pszPassword) * sizeof(TCHAR));
		SAFE_DELETE_ARRAY(pEntry->pszPassword);
		//pEntry->pszPassword = _UTF8ToString((UTF8_BYTE *)pData);
		pEntry->pszPassword = new char[strlen((char*)pData)+2] ;
		strcpy(pEntry->pszPassword, (char*) pData) ;
		
		break;
	case 0x0008:
		ASSERT(dwFieldSize != 0);
		SAFE_DELETE_ARRAY(pEntry->pszAdditional);
		//pEntry->pszAdditional = _UTF8ToString((UTF8_BYTE *)pData);
		pEntry->pszAdditional = new char[strlen((char*)pData)+2] ;
		strcpy(pEntry->pszAdditional, (char*) pData) ;
		break;
	case 0x0009:
		ASSERT(dwFieldSize == 5);
		memcpy(aCompressedTime, pData, 5);
		_TimeToPwTime(aCompressedTime, &pEntry->tCreation);
		break;
	case 0x000A:
		ASSERT(dwFieldSize == 5);
		memcpy(aCompressedTime, pData, 5);
		_TimeToPwTime(aCompressedTime, &pEntry->tLastMod);
		break;
	case 0x000B:
		ASSERT(dwFieldSize == 5);
		memcpy(aCompressedTime, pData, 5);
		_TimeToPwTime(aCompressedTime, &pEntry->tLastAccess);
		break;
	case 0x000C:
		ASSERT(dwFieldSize == 5);
		memcpy(aCompressedTime, pData, 5);
		_TimeToPwTime(aCompressedTime, &pEntry->tExpire);
		break;
	case 0x000D:
		ASSERT(dwFieldSize != 0);
		SAFE_DELETE_ARRAY(pEntry->pszBinaryDesc);
		//pEntry->pszBinaryDesc = _UTF8ToString((UTF8_BYTE *)pData);
		pEntry->pszBinaryDesc = new char[strlen((char*)pData)+2] ;
		strcpy(pEntry->pszBinaryDesc, (char*) pData) ;
		break;
	case 0x000E:
		SAFE_DELETE_ARRAY(pEntry->pBinaryData);
		if(dwFieldSize != 0)
		{
			pEntry->pBinaryData = new BYTE[dwFieldSize];
			memcpy(pEntry->pBinaryData, pData, dwFieldSize);
			pEntry->uBinaryDataLen = dwFieldSize;
		}
		break;
	case 0xFFFF:
		ASSERT(dwFieldSize == 0);
		AddEntry(pEntry);
		SAFE_DELETE_ARRAY(pEntry->pszTitle);
		SAFE_DELETE_ARRAY(pEntry->pszURL);
		SAFE_DELETE_ARRAY(pEntry->pszUserName);
		if(pEntry->pszPassword != NULL)
			mem_erase((unsigned char *)pEntry->pszPassword, _tcslen(pEntry->pszPassword) * sizeof(TCHAR));
		SAFE_DELETE_ARRAY(pEntry->pszPassword);
		SAFE_DELETE_ARRAY(pEntry->pszAdditional);
		SAFE_DELETE_ARRAY(pEntry->pszBinaryDesc);
		SAFE_DELETE_ARRAY(pEntry->pBinaryData);
		RESET_PWE_TEMPLATE(pEntry);
		break;
	default:
		return FALSE; // Field unsupported
	}

	return TRUE; // Field processed
}

void CPwManager::_GetNeverExpireTime(PW_TIME *pPwTime)
{
	ASSERT(pPwTime != NULL); if(pPwTime == NULL) return;
	memcpy(pPwTime, &g_pwTimeNever, sizeof(PW_TIME));
}

void CPwManager::FixGroupTree()
{
	DWORD i;
	USHORT usLastLevel = 0;

	m_pGroups[0].usLevel = 0; // First group must be root

	for(i = 0; i < m_dwNumGroups; i++)
	{
		if(m_pGroups[i].usLevel > (USHORT)(usLastLevel + 1))
			m_pGroups[i].usLevel = (USHORT)(usLastLevel + 1);

		usLastLevel = m_pGroups[i].usLevel;
	}
}

DWORD CPwManager::GetLastChildGroup(DWORD dwParentGroupIndex)
{
	DWORD i;
	USHORT usParentLevel;
	USHORT usLevel;
	USHORT usLastLevel;

	if(m_dwNumGroups <= 1) return 0;
	ASSERT(dwParentGroupIndex < m_dwNumGroups);
	if(dwParentGroupIndex == (m_dwNumGroups - 1)) return m_dwNumGroups - 1;
	else if(dwParentGroupIndex > (DWORD)(m_dwNumGroups - 1)) return DWORD_MAX;

	usParentLevel = m_pGroups[dwParentGroupIndex].usLevel;

	i = dwParentGroupIndex + 1;
	usLevel = usParentLevel;
	while(1)
	{
		usLastLevel = usLevel;
		usLevel = m_pGroups[i].usLevel;

		if(usLevel <= usParentLevel) return i - 1;

		i++;
		if(i >= m_dwNumGroups) break;
	}

	return DWORD_MAX;
}

BOOL CPwManager::AttachFileAsBinaryData(PW_ENTRY *pEntry, const TCHAR *lpFile)
{
	FILE *fp;
	DWORD dwFileLen;
	DWORD dwPathLen;
	LPTSTR pBinaryDesc;
	DWORD i;

	ASSERT_ENTRY(pEntry); if(pEntry == NULL) return FALSE;
	ASSERT(lpFile != NULL); if(lpFile == NULL) return FALSE;

	fp = _tfopen(lpFile, _T("rb"));
	if(fp == NULL) return FALSE;

	fseek(fp, 0, SEEK_END);
	dwFileLen = (DWORD)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if(dwFileLen == 0) { fclose(fp); fp = NULL; return FALSE; }
	ASSERT(dwFileLen > 0);

	SAFE_DELETE_ARRAY(pEntry->pszBinaryDesc);
	SAFE_DELETE_ARRAY(pEntry->pBinaryData);

	i = (DWORD)_tcslen(lpFile) - 1;
	while(1)
	{
		if(i == (DWORD)(-1)) break;
		if((lpFile[i] == '/') || (lpFile[i] == '\\')) break;
		i--;
	}
	pBinaryDesc = (LPTSTR)&lpFile[i + 1];

	dwPathLen = (DWORD)_tcslen(pBinaryDesc);

	pEntry->pszBinaryDesc = new TCHAR[dwPathLen + 1];
	_tcscpy(pEntry->pszBinaryDesc, pBinaryDesc);

	pEntry->pBinaryData = new BYTE[dwFileLen];
	fread(pEntry->pBinaryData, 1, dwFileLen, fp);

	pEntry->uBinaryDataLen = dwFileLen;

	fclose(fp); fp = NULL;
	return TRUE;
}

BOOL CPwManager::SaveBinaryData(PW_ENTRY *pEntry, const TCHAR *lpFile)
{
	FILE *fp;

	ASSERT_ENTRY(pEntry); if(pEntry == NULL) return FALSE;
	ASSERT(lpFile != NULL); if(lpFile == NULL) return FALSE;
	if(_tcslen(pEntry->pszBinaryDesc) == 0) return FALSE;

	fp = _tfopen(lpFile, _T("wb"));
	if(fp == NULL) return FALSE;

	if(pEntry->uBinaryDataLen != 0)
		fwrite(pEntry->pBinaryData, 1, pEntry->uBinaryDataLen, fp);

	fclose(fp); fp = NULL;
	return TRUE;
}

BOOL CPwManager::RemoveBinaryData(PW_ENTRY *pEntry)
{
	ASSERT_ENTRY(pEntry); if(pEntry == NULL) return FALSE;

	SAFE_DELETE_ARRAY(pEntry->pBinaryData);
	SAFE_DELETE_ARRAY(pEntry->pszBinaryDesc);
	pEntry->pszBinaryDesc = new TCHAR[1];
	pEntry->pszBinaryDesc[0] = 0;
	pEntry->uBinaryDataLen = 0;
	return TRUE;
}

void CPwManager::SubstEntryGroupIds(DWORD dwExistingId, DWORD dwNewId)
{
	DWORD i;

	ASSERT(dwExistingId != DWORD_MAX); ASSERT(dwNewId != DWORD_MAX);
	if(dwExistingId == dwNewId) return; // Nothing to do?

	for(i = 0; i < m_dwNumEntries; i++)
	{
		if(m_pEntries[i].uGroupId == dwExistingId)
			m_pEntries[i].uGroupId = dwNewId;
	}
}

// Encrypt the master key a few times to make brute-force key-search harder
BOOL CPwManager::_TransformMasterKey(BYTE *pKeySeed)
{
	Rijndael rijndael;
	RD_UINT8 aKey[32];
	RD_UINT8 aTest[16];
	RD_UINT8 aRef[16] = { // The Rijndael class will be tested, that's the expected ciphertext
		0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf,
		0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89
	};
	DWORD i;
	
	// kf: replaced sha256 class 
	///sha256_ctx sha2;
	sha256_context sha2 ;

	ASSERT(pKeySeed != NULL); if(pKeySeed == NULL) return FALSE;

	if(rijndael.init(Rijndael::ECB, Rijndael::Encrypt, (const RD_UINT8 *)pKeySeed,
		Rijndael::Key32Bytes, 0) != RIJNDAEL_SUCCESS)
	{
		return FALSE;
	}

	memcpy(m_pTransformedMasterKey, m_pMasterKey, 32);

	for(i = 0; i < m_dwKeyEncRounds; i++)
	{
		rijndael.blockEncrypt((const RD_UINT8 *)m_pTransformedMasterKey, 256, (RD_UINT8 *)m_pTransformedMasterKey);
	}

	// Do a quick test if the Rijndael class worked correctly
	for(i = 0; i < 32; i++) aKey[i] = (RD_UINT8)i;
	for(i = 0; i < 16; i++) aTest[i] = (RD_UINT8)(((RD_UINT8)i << 4) | (RD_UINT8)i);
	if(rijndael.init(Rijndael::ECB, Rijndael::Encrypt, aKey, Rijndael::Key32Bytes, NULL) != RIJNDAEL_SUCCESS)
		{ ASSERT(FALSE); return FALSE; }
	if(rijndael.blockEncrypt(aTest, 128, aTest) != 128) { ASSERT(FALSE); }
	if(memcmp(aTest, aRef, 16) != 0) { ASSERT(FALSE); return FALSE; }

	// Hash once with SHA-256
	sha256_starts(&sha2);
	sha256_update(&sha2, m_pTransformedMasterKey, 32);
	sha256_finish(&sha2, m_pTransformedMasterKey);

	// kf 
	//sha256_begin(&sha2);
	//sha256_hash(m_pTransformedMasterKey, 32, &sha2);
	//sha256_end(m_pTransformedMasterKey, &sha2);

	return TRUE;
}

DWORD CPwManager::GetKeyEncRounds()
{
	return m_dwKeyEncRounds;
}

void CPwManager::SetKeyEncRounds(DWORD dwRounds)
{
	// All allowed except DWORD_MAX
	if(dwRounds == DWORD_MAX) m_dwKeyEncRounds = DWORD_MAX - 1;
	else m_dwKeyEncRounds = dwRounds;
}

int CPwManager::DeleteLostEntries()
{
	DWORD i, dwEntryCount;
	BOOL bFixed = TRUE;
	PW_ENTRY *pe;
	PW_GROUP *pg;
	int iDeletedCount = 0;

	dwEntryCount = GetNumberOfEntries();
	if(dwEntryCount == 0) return 0;

	while(bFixed == TRUE)
	{
		bFixed = FALSE;

		for(i = 0; i < dwEntryCount; i++)
		{
			pe = GetEntry(i);
			ASSERT(pe != NULL); if(pe == NULL) break;

			pg = GetGroupById(pe->uGroupId);
			if(pg == NULL)
			{
				DeleteEntry(i);
				dwEntryCount--;
				bFixed = TRUE;
				iDeletedCount++;
				break;
			}
		}
	}

	return iDeletedCount;
}

BOOL CPwManager::IsAllowedStoreGroup(LPCTSTR lpGroupName, LPCTSTR lpSearchGroupName)
{
	ASSERT(lpGroupName != NULL); if(lpGroupName == NULL) return FALSE;

	if(_tcscmp(lpGroupName, lpSearchGroupName) == 0) return FALSE;
	return TRUE;
}

///BOOL CPwManager::BackupEntry(const PW_ENTRY *pe, BOOL *pbGroupCreated)
///{
	/*
	PW_ENTRY pwe;
	PW_GROUP pwg;
	DWORD dwGroupId;

	ASSERT(pe != NULL); if(pe == NULL) return FALSE;

	if(pbGroupCreated != NULL) *pbGroupCreated = FALSE;

	dwGroupId = GetGroupId(PWS_BACKUPGROUP);
	if(dwGroupId == DWORD_MAX)
	{
		ZeroMemory(&pwg, sizeof(PW_GROUP));
		pwg.pszGroupName = (TCHAR *)PWS_BACKUPGROUP;
		_GetCurrentPwTime(&pwg.tCreation);
		pwg.tLastAccess = pwg.tCreation;
		pwg.tLastMod = pwg.tCreation;
		_GetNeverExpireTime(&pwg.tExpire);
		pwg.uImageId = 4;

		if(AddGroup(&pwg) == FALSE) return FALSE;
		if(pbGroupCreated != NULL) *pbGroupCreated = TRUE;
		dwGroupId = GetGroupId(PWS_BACKUPGROUP);
	}
	if(dwGroupId == DWORD_MAX) return FALSE;

	pwe = *pe;
	_GetCurrentPwTime(&pwe.tLastMod);
	pwe.uGroupId = dwGroupId;
	ZeroMemory(&pwe.uuid, 16);

	return AddEntry(&pwe);
	*/
//
//	return FALSE ;
//}

/* DWORD CPwManager::MakeGroupTree(LPCTSTR lpTreeString, TCHAR tchSeparator)
{
	DWORD i, j = 0, dwCurGroup = 0, dwTestGroup;
	DWORD dwId = DWORD_MAX;
	TCHAR *ptzTemp;
	DWORD dwStrLen;
	DWORD dwLevel = 0;
	TCHAR tch;

	ASSERT(lpTreeString != NULL); if(lpTreeString == NULL) return DWORD_MAX;

	dwStrLen = (DWORD)_tcslen(lpTreeString);
	ptzTemp = new TCHAR[dwStrLen + 1]; ASSERT(ptzTemp != NULL);
	ptzTemp[0] = 0;

	for(i = 0; i < (dwStrLen + 1); i++)
	{
		tch = lpTreeString[i];

		if((tch == tchSeparator) || (tch == 0))
		{
			ptzTemp[j] = 0;

			j = 0;
		}
		else
		{
			ptzTemp[j] = tch;
			j++;
		}
	}

	SAFE_DELETE_ARRAY(ptzTemp);
	return dwId;
} */

/*
 std::string CPwManager::FormatError(int nErrorCode, DWORD dwFlags)
{
	std::string str;
	TCHAR tszTemp[24];

	_stprintf(tszTemp, _T("%08X"), (unsigned int)nErrorCode);

	if((dwFlags & PWFF_NO_INTRO) == 0)
	{
		//str = TRL("An error occured"); str += _T("!\r\n");
		str = _T("An error occured"); str += _T("!\r\n");
	}

	str += _T("Error code"); str += _T(": 0x");
	//str += TRL("Error code"); str += _T(": 0x");
	
	str += tszTemp;

	if((dwFlags & PWFF_NO_INTRO) == 0) str += _T("\r\n\r\n");
	else str += _T("\r\n");

	// kf !!!

	return str;
}
*/

BOOL CPwManager::_AddMetaStream(LPCTSTR lpMetaDataDesc, BYTE *pData, DWORD dwLength)
{
	PW_ENTRY pe;

	ASSERT(lpMetaDataDesc != NULL); ASSERT(_tcslen(lpMetaDataDesc) != 0);
	ASSERT(pData != NULL); if(pData == NULL) return FALSE;
	ASSERT(dwLength != 0); if(dwLength == 0) return TRUE;

	memset(&pe, 0, sizeof(PW_ENTRY));
	pe.pBinaryData = pData; pe.pszAdditional = (TCHAR *)lpMetaDataDesc;
	pe.pszBinaryDesc = PMS_ID_BINDESC; pe.pszPassword = _T("");
	pe.pszTitle = PMS_ID_TITLE; pe.pszURL = PMS_ID_URL;
	pe.pszUserName = PMS_ID_USER;
	pe.tCreation = g_pwTimeNever; pe.tExpire = g_pwTimeNever;
	pe.tLastAccess = g_pwTimeNever; pe.tLastMod = g_pwTimeNever;
	pe.uBinaryDataLen = dwLength;
	if(GetNumberOfGroups() != 0) pe.uGroupId = m_pGroups[0].uGroupId;

	return AddEntry(&pe);
}

BOOL CPwManager::_IsMetaStream(PW_ENTRY *p)
{
	ASSERT_ENTRY(p); if(p == NULL) return FALSE;

	if(p->pBinaryData == NULL) return FALSE;
	if(p->pszAdditional == NULL) return FALSE;
	if(p->pszBinaryDesc == NULL) return FALSE;
	if(_tcscmp(p->pszBinaryDesc, PMS_ID_BINDESC) != 0) return FALSE;
	if(p->pszTitle == NULL) return FALSE;
	if(_tcscmp(p->pszTitle, PMS_ID_TITLE) != 0) return FALSE;
	if(p->pszUserName == NULL) return FALSE;
	if(_tcscmp(p->pszUserName, PMS_ID_USER) != 0) return FALSE;
	if(p->pszURL == NULL) return FALSE;
	if(_tcscmp(p->pszURL, PMS_ID_URL) != 0) return FALSE;
	if(p->uImageId != 0) return FALSE;

	return TRUE;
}

DWORD CPwManager::_LoadAndRemoveAllMetaStreams()
{
	BOOL bChange = TRUE;
	DWORD i;
	DWORD dwEntryCount;
	PW_ENTRY *p;
	DWORD dwMetaStreamCount = 0;

	if(m_pEntries == NULL) return 0;
	if(m_pGroups == NULL) return 0;
	if(GetNumberOfEntries() == 0) return 0;
	if(GetNumberOfGroups() == 0) return 0;

	while(bChange == TRUE)
	{
		bChange = FALSE;
		dwEntryCount = GetNumberOfEntries();
		if(dwEntryCount == 0) break;

		i = dwEntryCount - 1;
		while(1)
		{
			p = GetEntry(i);
			if(_IsMetaStream(p) == TRUE)
			{
				_ParseMetaStream(p);
				//VERIFY(DeleteEntry(i));
				DeleteEntry(i);
				dwMetaStreamCount++;
				bChange = TRUE;
				break;
			}

			if(i == 0) break;
			i--;
		}
	}

	return dwMetaStreamCount;
}

BOOL CPwManager::_AddAllMetaStreams()
{
	PMS_SIMPLE_UI_STATE simpState;
	BOOL b = TRUE;

	memset(&simpState, 0, sizeof(PMS_SIMPLE_UI_STATE));

	simpState.uLastSelectedGroupId = m_dwLastSelectedGroupId;
	simpState.uLastTopVisibleGroupId = m_dwLastTopVisibleGroupId;
	memcpy(simpState.aLastSelectedEntryUuid, m_aLastSelectedEntryUuid, 16);
	memcpy(simpState.aLastTopVisibleEntryUuid, m_aLastTopVisibleEntryUuid, 16);

	b &= _AddMetaStream(PMS_STREAM_SIMPLESTATE, (BYTE *)&simpState, sizeof(PMS_SIMPLE_UI_STATE));

	ASSERT(b == TRUE); return b;
}

void CPwManager::_ParseMetaStream(PW_ENTRY *p)
{
	PMS_SIMPLE_UI_STATE *pState;

	ASSERT(_IsMetaStream(p) == TRUE);

	if(_tcscmp(p->pszAdditional, PMS_STREAM_SIMPLESTATE) == 0)
	{
		pState = (PMS_SIMPLE_UI_STATE *)p->pBinaryData;

		m_dwLastSelectedGroupId = pState->uLastSelectedGroupId;
		m_dwLastTopVisibleGroupId = pState->uLastTopVisibleGroupId;
		memcpy(m_aLastSelectedEntryUuid, pState->aLastSelectedEntryUuid, 16);
		memcpy(m_aLastTopVisibleEntryUuid, pState->aLastTopVisibleEntryUuid, 16);
	}
}
