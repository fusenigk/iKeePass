//
//  RootViewController.h
//  TestKeePass
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
#import "ChildViewController.h"
//#import "GroupViewController.h"

@interface RootViewController : UITableViewController {
	NSString *databaseName;
	NSMutableArray *tree;
	NSDictionary *group;
	UINavigationController *navigationController;
}

@property (readwrite, retain) NSMutableArray *tree;
@property (readwrite, retain) NSDictionary *group;
@property (readwrite, retain) NSString *databaseName;
@property (nonatomic, retain) IBOutlet UINavigationController *navigationController;

- (UITableViewCell *)tableviewCellWithReuseIdentifier:(NSString *)identifier;
- (void)configureCell:(UITableViewCell *)cell forIndexPath:(NSIndexPath *)indexPath;

@end
