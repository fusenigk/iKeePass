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

#ifndef ___KEEPASS_PLUGIN_DEF_H___
#define ___KEEPASS_PLUGIN_DEF_H___

//#include <windows.h>
#include "SysDefEx.h"

#define KP_I_INIT "KeePluginInit"
#define KP_I_CALL "KeePluginCall"
#define KP_I_EXIT "KeePluginExit"

//////////////////////////////////////////////////////////////////////////
// KeePass menu item flags

#define KPMIF_NORMAL      0
#define KPMIF_CHECKBOX    1
#define KPMIF_ENABLED     0
#define KPMIF_DISABLED    2
#define KPMIF_POPUP_START 4
#define KPMIF_POPUP_END   8
// To make a separator, set lpCommandString to ""

//////////////////////////////////////////////////////////////////////////
// KeePass menu item states

#define KPMIS_UNCHECKED   0
#define KPMIS_CHECKED     1

//////////////////////////////////////////////////////////////////////////
// KeePass plugin structures

typedef struct
{
	DWORD dwAppVersion; // 0.98b would be 0x00090802
	HWND hwndMain;
	void *pApp;     // Pointer to current CPwSafeApp class, cast it
	void *pMainDlg; // Pointer to current CPwSafeDlg class, cast it
	void *pPwMgr;   // Pointer to current CPwManager class, cast it
} KP_APP_INFO, *LPKP_APP_INFO;

typedef struct
{
	DWORD dwFlags; // See above, KPMIF_XXX flags
	DWORD dwState; // See above, KPMIS_XXX flags
	DWORD dwIcon;
	TCHAR *lpCommandString;
	DWORD dwCommandID; // Set by KeePass, don't modify yourself
} KP_MENU_ITEM;

typedef struct
{
	DWORD dwForAppVer; // Plugin has been designed for this KeePass version
	DWORD dwPluginVer;
	TCHAR tszPluginName[64];
	TCHAR tszAuthor[64]; // Author of the plugin

	DWORD dwNumCommands; // Number of menu items
	KP_MENU_ITEM *pMenuItems;
} KP_PLUGIN_INFO, *LPKP_PLUGIN_INFO;

//////////////////////////////////////////////////////////////////////////
// Functions called by KeePass (must be exported by the plugin DLL)

// KP_EXP BOOL KP_API KeePluginInit(const KP_APP_INFO *pAppInfo, KP_PLUGIN_INFO *pPluginInfo);
typedef BOOL(KP_API *LPKEEPLUGININIT)(const KP_APP_INFO *pAppInfo, KP_PLUGIN_INFO *pPluginInfo);

// KP_EXP BOOL KP_API KeePluginCall(DWORD dwCode, LPARAM lParamW, LPARAM lParamL);
typedef BOOL(KP_API *LPKEEPLUGINCALL)(DWORD dwCode, LPARAM lParamW, LPARAM lParamL);

// KP_EXP BOOL KP_API KeePluginExit(LPARAM lParamW, LPARAM lParamL);
typedef BOOL(KP_API *LPKEEPLUGINEXIT)(LPARAM lParamW, LPARAM lParamL);

//////////////////////////////////////////////////////////////////////////
// The structure that holds all information about one single plugin

typedef struct
{
	DWORD dwPluginID; // Assigned by KeePass, used internally
	TCHAR tszFile[MAX_PATH];
	BOOL bEnabled;
	HINSTANCE hinstDLL;

	KP_PLUGIN_INFO info;

	LPKEEPLUGININIT lpInit;
	LPKEEPLUGINCALL lpCall;
	LPKEEPLUGINEXIT lpExit;
} KP_PLUGIN_INSTANCE, *LPKP_PLUGIN_INSTANCE;

//////////////////////////////////////////////////////////////////////////
// KeePass query IDs (used in function KP_Query)

#define KPQUERY_NULL    0
#define KPQUERY_VERSION 1

//////////////////////////////////////////////////////////////////////////
// KeePass plugin message codes

#define KPM_NULL 0
#define KPM_DIRECT_EXEC 1
#define KPM_DIRECT_CONFIG 2
#define KPM_PLUGIN_INFO 3

// General notifications

#define KPM_INIT_MENU_POPUP 4

#define KPM_WND_INIT_POST 6

#define KPM_DELETE_TEMP_FILES_PRE 7
#define KPM_WM_CANCEL 12

#define KPM_PWLIST_RCLICK 18
#define KPM_GROUPLIST_RCLICK 20

#define KPM_OPENDB_PRE 25
#define KPM_OPENDB_POST 26

#define KPM_SAVEDB_POST 49

// File menu commands

#define KPM_FILE_NEW_PRE 23
#define KPM_FILE_NEW_POST 24
#define KPM_FILE_OPEN_PRE 27
#define KPM_FILE_SAVE_PRE 28
#define KPM_FILE_SAVEAS_PRE 29
#define KPM_FILE_CLOSE_PRE 30

#define KPM_FILE_PRINT_PRE 35
#define KPM_FILE_PRINTPREVIEW_PRE 45

#define KPM_FILE_DBSETTINGS_PRE 48
#define KPM_FILE_CHANGE_MASTER_KEY_PRE 34

#define KPM_FILE_LOCK_PRE 42
#define KPM_FILE_EXIT_PRE 8

// Other menu commands

#define KPM_OPTIONS_PRE 31
#define KPM_OPTIONS_POST 32
#define KPM_VIEW_HIDE_STARS_PRE 11
#define KPM_GEN_PASSWORD_PRE 36
#define KPM_TANWIZARD_PRE 44
#define KPM_INFO_ABOUT_PRE 9

// Password list commands

#define KPM_ADD_ENTRY_PRE 5
#define KPM_ADD_ENTRY 14
#define KPM_EDIT_ENTRY_PRE 15
// #define KPM_EDIT_ENTRY 16
#define KPM_DELETE_ENTRY_PRE 17
#define KPM_DUPLICATE_ENTRY_PRE 40

#define KPM_PWLIST_FIND_PRE 38
#define KPM_PWLIST_FIND_IN_GROUP_PRE 39

#define KPM_MASSMODIFY_ENTRIES_PRE 43

// Direct entry commands

#define KPM_PW_COPY 19
#define KPM_USER_COPY 21
#define KPM_URL_VISIT 22

// Group list commands

#define KPM_GROUP_ADD_PRE 10
#define KPM_GROUP_ADD 13
#define KPM_GROUP_ADD_SUBGROUP_PRE 46
#define KPM_GROUP_MODIFY_PRE 37
#define KPM_GROUP_REMOVE_PRE 33
#define KPM_GROUP_SORT_PRE 47
#define KPM_GROUP_PRINT_PRE 41

// The following is unused. It's always the last command ID + 1
#define KPM_NEXT 50

#endif
