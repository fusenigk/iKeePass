//
//  iKeePassViewController.h
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

@interface iKeePassViewController : UIViewController <UIPickerViewDelegate>  {
	UIWindow *window;
    UINavigationController *navigationController;
	
	IBOutlet UIPickerView *filePicker;
	IBOutlet UILabel *sizeLabel;
	IBOutlet UILabel *serverLabel;
	IBOutlet UILabel *useLocalDBLabel;
	IBOutlet UIToolbar *toolBar;
	IBOutlet UINavigationBar *navBar;
	UITextField *myTextField;
	NSString *textFieldString;
	IBOutlet UIBarButtonItem *openButton;
	IBOutlet UIBarButtonItem *editButton;
	IBOutlet UIBarButtonItem *addButton;
	IBOutlet UIBarButtonItem *deleteButton;
	IBOutlet UIBarButtonItem *aboutButton;
	IBOutlet UITextField *serverTextField;
	IBOutlet UISwitch *useLocalDBSwitch;
	NSMutableDictionary *fileList;
	
	id HUD;
	
	
	NSMutableArray *fileArray;	
	BOOL toggleServerField;
	NSString *alertType;
	BOOL dataBaseLoadedSuccessful;
}

@property (nonatomic, retain) NSMutableDictionary *fileList;
@property (nonatomic, retain) NSString *textFieldString;
@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UINavigationController *navigationController;


- (IBAction )openDatabaseFile:(id) sender ;
- (IBAction )deleteDatabaseFile:(id) sender ;
- (IBAction )editServerField:(id) sender ;
- (IBAction )addDatabaseFromServer:(id) sender ;
- (IBAction )about:(id) sender ;
- (BOOL) isNetworkAvailable:(NSString* )databaseAtServerURL;
-(void) openDatabase:(NSString* )pathToDatabase andDatabaseName:(NSString *)databaseName withPassword:(NSString*) passwordForDatabase;
-(void) fetchDatabaseFromServer:(NSString* )databaseAtServerURL andDatabaseName:(NSString *)databaseName withUserName:(NSString*) userName andPassword:(NSString *) password;
-(void) checkNetwork;
-(void) close;

- (NSString *) fileInformationForSelectedFile:(NSInteger )selectedFileNum;
- (NSString *) serverURLforSelectedFile: (NSInteger )selectedFileNum;
- (void) reloadFilePicker;

@end

