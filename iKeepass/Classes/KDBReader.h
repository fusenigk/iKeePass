//
//  KDBReader.h
//  iKeePass
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

#import <UIKit/UIKit.h>


@interface KDBReader : NSObject {
	NSMutableArray *tree;
	
	NSMutableArray *currentChildren;
	NSMutableArray *groupStack;
	NSMutableArray *entryStack;
	
	NSInteger numGroup;
	NSInteger numEntry;
	
	NSMutableDictionary *currentGroup;
	
	short level;
	short lastLevel;
	short currentLevel;
	
	NSMutableArray			*menuPWList;
	
	unsigned long  dwPWGroupIndex ;
	
	NSString * stPWGroupName ;
}
@property (readwrite, retain) NSMutableArray *tree;
@property (readwrite, retain) NSMutableArray *groupStack;
@property (readwrite, retain) NSMutableArray *entryStack;
@property (readwrite, retain) NSMutableArray *currentChildren;
@property (readwrite, retain) NSMutableDictionary *currentGroup;

- (BOOL)parseKDBFileAtURL:(NSString *)URL withPassword:(NSString *) password;
NSMutableArray* parseKDBFileAtURL(NSString *URL, NSString *password);

- (void) setPWGroupdId: (unsigned long) z ;
- (unsigned long) getPWGroupId;

- (void) setPWGroupName: (NSString*) name ;
- (NSString*) getPWGroupName;
@end
