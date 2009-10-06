//
//  KDBReader.m
//  iKeePass
//
/*
  iKeePass Password Safe - The Open-Source Password Manager
  Copyright (C) 2008-2009 Markus Schlecht and Karsten Fusenig
  
  
  based on KeePass
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

#import "KDBReader.h"

#import "StdAfx.h"
#import "PwManager.h"
#import <Foundation/Foundation.h>


@implementation KDBReader



CPwManager m_mgr;

char stSelected[100] ="";
char szDatabaseFileName[1024] ="";

char szUsername[1024] ="";
char szPassword[1024] ="";

bool bFirst = true ;


@synthesize currentChildren;
@synthesize groupStack;
@synthesize entryStack;
@synthesize tree;
@synthesize currentGroup;

-(id)init {
	if (self = [super init]) {
		// NSLog(@"XMLReader init");
		numGroup = 0;
		numEntry = 0;
		level = -1;
		lastLevel = -1;
		currentLevel = -1;
		groupStack = [[NSMutableArray alloc] init];
	}
	return self;
}


- (BOOL)parseKDBFileAtURL:(NSString *)URL withPassword:(NSString *) password {
	
	NSMutableArray *mTreeList = [[NSMutableArray alloc] init];
	int nErr = 0 ;
	
	if (strcmp(szUsername, "") == 0)
	{
	}
	
	// /User/q176197/Daten.kdb
	NSString *stLocalDestinationFile = URL;
	
	NSString *stPassword = @"" ;
	if ([password length] == 0 ) {
		UIAlertView *alert= [[UIAlertView alloc] initWithTitle:@"iKeePass" message:NSLocalizedString(@"MISSING_PASSWORD_MESSAGE", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil];
		[alert show];
		[alert release];	
		return NO;
	}
	stPassword=[stPassword stringByAppendingString:password];
	
	if ([stPassword length] > 0)
		strcpy(szPassword, (const char*) [stPassword fileSystemRepresentation]) ;
	
	nErr = m_mgr.SetMasterKey(szPassword, 0, NULL, NULL, FALSE);
	
	// todo: check max. array size !
	char szFileName[1024] = "" ;
	strcpy(szFileName, (const char*) [stLocalDestinationFile fileSystemRepresentation]) ;
	 // Datenbank nun öffnen
	
	nErr = m_mgr.OpenDatabase(szFileName, NULL);
	
	if (nErr != 1)
	{
		//NSLog(@"Fehler beim DB Öffnen! ");
		//printf("Error Code: %d \n", nErr) ;
		
		if (nErr == PWE_INVALID_KEY)
		{
			// falsches Passwort
			UIAlertView *alert= [[UIAlertView alloc] initWithTitle:@"iKeePass" message:NSLocalizedString(@"WRONG_PASSWORD_MESSAGE", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil];
			[alert show];
			[alert release];	
			return NO;
		}
		else if (nErr == PWE_NOFILEACCESS_READ)
			{
				UIAlertView *alert= [[UIAlertView alloc] initWithTitle:@"iKeePass" message:NSLocalizedString(@"COULD_NOT_LOAD_DB_MESSAGE", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil];
				[alert show];
				[alert release];
				return NO;
			}
		else if (nErr == PWE_INVALID_FILESIGNATURE)
		{
			UIAlertView *alert= [[UIAlertView alloc] initWithTitle:@"iKeePass" message:NSLocalizedString(@"CORRUPTED_DB_MESSAGE", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil];
			[alert show];
			[alert release];
			return NO;
		}
		else if (nErr == PWE_INVALID_FILEHEADER)
		{
			UIAlertView *alert= [[UIAlertView alloc] initWithTitle:@"iKeePass" message:NSLocalizedString(@"ERROR_NONETWORK", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil];
			[alert show];
			[alert release];
			return NO;
		}
			else
			{
				UIAlertView *alert= [[UIAlertView alloc] initWithTitle:@"iKeePass" message:NSLocalizedString(@"UNKNOWN_ERROR_MESSAGE", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil];
				[alert show];
				[alert release];
				
				return NO;
			}
		
		// cancel loading of the iKeePass application //
		//return ;
	}
	
	
	PW_GROUP *pgrp;
	int i = 0 ;
	
	//printf("Anzahl Gruppen: %d\n", m_mgr.GetNumberOfGroups());
	//printf("Anzahl Einträge: %d\n", m_mgr.GetNumberOfEntries());
	
	NSMutableDictionary *group;
	NSMutableDictionary *entry;
	
	for(i = 0; i < m_mgr.GetNumberOfGroups(); i++)
	{
		pgrp = m_mgr.GetGroup(i);
		currentLevel = pgrp->usLevel;
		
		// Element für Gruppe erzeugen !!!
		group = [[[NSMutableDictionary alloc] init] retain];
		[group setObject:[NSNumber numberWithBool:YES] forKey:@"group"];
		
		// Erzeuge gleich das ChildrenArray
		NSMutableArray *children = [[[NSMutableArray alloc] init] retain];
		// Füge das Childrenarray zur Gruppe hinzu
		[group setObject:children forKey:@"children"];
		
		NSString *groupName = [[NSString alloc] initWithCString:pgrp->pszGroupName encoding:NSUTF8StringEncoding];
		[group setObject:groupName forKey:@"title"];
		NSString *iconName = [NSString stringWithFormat:@"%u", pgrp->uImageId];
		[group setObject:iconName forKey:@"icon"];
		
		currentChildren = children;
		
		//printf("Log: Groupname: %s - group-d: %u  Level: %d Icon: %u\n", pgrp->pszGroupName, pgrp->uGroupId, pgrp->usLevel, pgrp->uImageId) ;
		//NSLog(@"Log: Level: %i lastLevel: %i", level, lastLevel);
		
		int gi = 0;
		int ii = 0;
		
		gi = m_mgr.GetNumberOfItemsInGroupN(pgrp->uGroupId);
		for (ii=0; ii< gi; ii++) {
			entry = [[[NSMutableDictionary alloc] init] retain];
			[entry setObject:[NSNumber numberWithBool:YES] forKey:@"entry"];
			if (level = 0)
				[mTreeList addObject:entry];
			else 
				[[group objectForKey:@"children"] addObject:entry];
			
			
			
			PW_ENTRY	*pentry;
			
			pentry = m_mgr.GetEntryByGroup(pgrp->uGroupId, ii);
			//printf(" Name (%u): %s\n", pentry->uGroupId, pentry->pszTitle);
			
			NSString *entryName = [[NSString alloc] initWithCString:pentry->pszTitle encoding:NSUTF8StringEncoding];
			[entry setObject:entryName forKey:@"title"];
			NSString *entryIconName = [NSString stringWithFormat:@"%u", pentry->uImageId];
			[entry setObject:entryIconName forKey:@"icon"];
			// username
			NSString *userName = [[NSString alloc] initWithCString:pentry->pszUserName encoding:NSUTF8StringEncoding];
			[entry setObject:userName forKey:@"username"];
			// url
			NSString *urlString = [[NSString alloc] initWithCString:pentry->pszURL encoding:NSUTF8StringEncoding];
			[entry setObject:urlString forKey:@"url"];
			// comment
			NSString *comment = [[NSString alloc] initWithCString:pentry->pszAdditional encoding:NSUTF8StringEncoding];
			[entry setObject:comment forKey:@"comment"];
			// creation
			//NSString *creation = [NSString stringWithFormat:@"%u", pentry->tCreation];
			NSString *creation = [NSString stringWithFormat:@"%@", [self parsePWTimeToNSString:pentry->tCreation]];
			[entry setObject:creation forKey:@"creation"];
			// lastaccess
			NSString *lastaccess = [NSString stringWithFormat:@"%@", [self parsePWTimeToNSString:pentry->tLastAccess]];
			[entry setObject:lastaccess forKey:@"lastaccess"];
			 
			// lastmod
			NSString *lastmod = [NSString stringWithFormat:@"%@", [self parsePWTimeToNSString:pentry->tLastMod]];
			[entry setObject:lastmod forKey:@"lastmod"];
			// expire
			NSString *expire = [NSString stringWithFormat:@"%@", [self parsePWTimeToNSString:pentry->tExpire]];
			//NSString *expire = [NSString stringWithFormat:@"%u", pentry->tExpire];
			
			[entry setObject:expire forKey:@"expire"];
			
			m_mgr.UnlockEntryPassword(pentry);
			//printf(" PW: %s\n", pentry->pszPassword);
			NSString *entryPassword = [[NSString alloc] initWithCString:pentry->pszPassword encoding:NSUTF8StringEncoding];
			[entry setObject:entryPassword forKey:@"password"];
						
			m_mgr.LockEntryPassword(pentry);
		}
		
		if (currentLevel <=0) {
			// Die Gruppe zur TreeListe hinzufügen
			[mTreeList addObject:group];
			// Falls noch Gruppen auf dem gruppenstack sind, alle entfernen
			if (groupStack.count > 0)
				[groupStack removeAllObjects];
			// aktuelles Element zum Groupstack hinzufügen
			[groupStack addObject:group];
		} else {
			if (lastLevel < currentLevel) {
				// Level erhöht sich
				//printf("Log: Level wurde auf %u erhoeht\n", currentLevel);
				// Element zu dem letzten Object auf dem Stack hinzufügen!
				[[[groupStack lastObject] objectForKey:@"children"] addObject:group];
				// Diese Gruppe auf den Stack drauf packen
				[groupStack addObject:group];
			} else if (lastLevel == currentLevel) {
				// Level bleibt gleich
				//printf("Log: Level verbleibt bei %u\n", currentLevel);
				if (groupStack.count > currentLevel) 
					[groupStack removeLastObject];
				// Element zu dem letzten Object auf dem Stack hinzufügen!
				[[[groupStack lastObject] objectForKey:@"children"] addObject:group];
				// Diese Gruppe auf den Stack drauf packen
				[groupStack addObject:group];
			} else if (lastLevel > currentLevel) {
				// Level wird kleiner, um wieviel?
				int decrement = (lastLevel - currentLevel + 1);
				//printf("Log: Level wurde um %u verkleinert\n", decrement);
				for (int i=0; i<decrement; i++) {
					[groupStack removeLastObject];
				}
				[[[groupStack lastObject] objectForKey:@"children"] addObject:group];
			}
		
		}
		
		//printf("Log: groupStack.count: %u \n", groupStack.count) ;
		//printf("Log: Level: %d - lastLevel: %d - currentLevel: %d\n", level, lastLevel, currentLevel) ;
		lastLevel = currentLevel;
		
	}	
	tree = mTreeList;
	return YES;
}

-(NSString *) parsePWTimeToNSString:(PW_TIME) pw_time {
	
	//NSTimeZone *timeZone = [NSTimeZone localTimeZone];
	NSString *datum = @"never";
	
	if (pw_time.shYear == nil)
		datum = NSLocalizedString(@"UNDEFINED", @"");
	else {
		if (pw_time.shYear == 2999) {
			datum = NSLocalizedString(@"NEVER_DATE", @"");
		} else {
			NSDateComponents *comps = [[NSDateComponents alloc] init];
			[comps setDay:pw_time.btDay];
			[comps setMonth:pw_time.btMonth];
			[comps setYear:pw_time.shYear];
			[comps setHour:pw_time.btHour];
			[comps setMinute:pw_time.btMinute];
			[comps setSecond:pw_time.btSecond];
			NSCalendar *gregorian = [[NSCalendar alloc]
									 initWithCalendarIdentifier:NSGregorianCalendar];
			[gregorian setTimeZone:[NSTimeZone localTimeZone]];
			NSDate *date = [gregorian dateFromComponents:comps];
			[comps release];
			
			NSDateFormatter *dateFormatter = [[[NSDateFormatter alloc] init]  autorelease];
			[dateFormatter setDateStyle:NSDateFormatterMediumStyle];
			[dateFormatter setTimeStyle:NSDateFormatterShortStyle];
			
			//NSLog(@"LocalTimeZone: %@ with Date:%@", [timeZone description], currentDate );
			datum = [[NSString stringWithFormat:@"%@", [dateFormatter stringFromDate:date]] retain];
			
			[gregorian release];
			
		}
	}
	
	
	return  datum;
}

- (void) setPWGroupdId: (unsigned long) z 
{
    dwPWGroupIndex = z;
}

- (unsigned long) getPWGroupId
{
    return dwPWGroupIndex;
}

- (void)dealloc {
	
	[menuPWList release];
	
	[super dealloc];
}

- (void) setPWGroupName: (NSString*) name
{
	stPWGroupName = name ;
};

- (NSString*) getPWGroupName{
	return stPWGroupName ;
};

@end
