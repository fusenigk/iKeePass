//
//  iKeePassViewController.m
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

#import "iKeePassViewController.h"
#import "XMLReader.h"
#import "KDBReader.h"
#import "RootViewController.h"
#import <SystemConfiguration/SCNetworkReachability.h>

@interface UIProgressHUD : NSObject
- (void) show: (BOOL) yesOrNo;
- (UIProgressHUD *) initWithWindow: (UIView *) window;
@end


@implementation iKeePassViewController

- (void) killHUD: (id) aHUD
{
	[aHUD show:NO];
	[aHUD release];
}
- (void) presentSheet
{
	HUD = [[UIProgressHUD alloc] initWithWindow:[self.view superview]];
    [HUD setText:NSLocalizedString(@"DOWNLOADING",@"Downloading File. Please wait.")];
    [HUD show:YES];
	
    //[self performSelector:@selector(killHUD:) withObject:HUD afterDelay:5.0];
}



@synthesize window; 
@synthesize fileList;
@synthesize navigationController;
@synthesize textFieldString;

#define iKeePassDatabaseFiles @"iKeePassDatabaseFiles.plist" 


- (IBAction )about:(id) sender {
	// Display about Message
	UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"iKeePass", @"") message:NSLocalizedString(@"ABOUT_MESSAGE", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil, nil];
	[myAlertView show];
	[myAlertView release];
}

- (IBAction )openDatabaseFile:(id) sender {
	if (fileArray.count > 0) {
		//NSLog(@"Öffnen");
		alertType = @"PASSWORD";
		// Passwort abfragen!
		UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"ASK_FOR_PASSWORD", @"") message:NSLocalizedString(@"\n", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:NSLocalizedString(@"OK_BUTTON", @""), nil];
		myTextField = [[UITextField alloc] initWithFrame:CGRectMake(12.0, 45.0, 260.0, 25.0)];
		[myTextField setBackgroundColor:[UIColor whiteColor]];
		myTextField.secureTextEntry = YES;
		[myAlertView addSubview:myTextField];
		
		
		CGAffineTransform myTransform = CGAffineTransformMakeTranslation(0.0, 130.0);
		[myAlertView setTransform:myTransform];
		//myTextField.keyboardAppearance = UIKeyboardAppearanceAlert;
		
		[myAlertView show];
		[myAlertView release];
		
		// Damit wird gleich das Keyboard in den Eingabemodus geschaltet
		[myTextField becomeFirstResponder];
	}
}

- (IBAction )deleteDatabaseFile:(id) sender {
	// Falls eine DB-Datei selektiert is einen Alert einblenden, ob die Datei wirklich gelöscht werden soll!
	if (fileArray.count > 0) {
		alertType = @"DELETE";
		NSString *fileName = [NSString stringWithFormat:NSLocalizedString(@"ASK_REALLY_DELETE_DATABASE", @""), [fileArray objectAtIndex:[filePicker selectedRowInComponent:0]]];
		UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"iKeePass" message:fileName delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:NSLocalizedString(@"DELETE_BUTTON", @""), nil];
		[myAlertView show];
		[myAlertView release];
	}
}

- (IBAction )editServerField:(id) sender {
	//NSLog(@"Edit");
	if (!toggleServerField) {
		//NSLog(@"Editiere");
		toggleServerField = YES;
		serverTextField.enabled = YES;
		editButton.title = NSLocalizedString(@"DONE_BUTTON", @"");
		editButton.style = UIBarButtonItemStyleDone;
		// gleich Keyboard öffnen !!!
		[serverTextField becomeFirstResponder];
		
	} else {
		//NSLog(@"Done");
		toggleServerField = NO;
		serverTextField.enabled = NO;
		// neue ServerURL speichern!
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults]; 
		[defaults setValue:serverTextField.text forKey:@"IKEEPASS_SERVER_URL"];
		
		editButton.title = NSLocalizedString(@"EDIT_BUTTON", @"");
		//editButton.style = UIBarButtonItemStyle;
	}
}

- (void)alertView:(UIAlertView *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
	// the user clicked one of the OK/Cancel buttons
	if (buttonIndex == 0)
	{
		//NSLog(@"cancel");
	}
	else
	{
		textFieldString = myTextField.text;
		if ( [alertType isEqualToString:@"PASSWORD"]) {
			if (useLocalDBSwitch.on) {
			
				// Passwort überprüfen!!
				//if ([myTextField.text isEqualToString:@"Hallo"]) {
				//[self openApplication];
				NSString *fileName = [fileArray objectAtIndex:[filePicker selectedRowInComponent:0]];
		
				NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
				NSString *documentsDirectory = [paths objectAtIndex:0];
			
				NSString *path = [documentsDirectory stringByAppendingPathComponent:fileName];
		
				[self openDatabase:path andDatabaseName:fileName withPassword:textFieldString];
				//NSLog(@"Open local Database: %@", path);
			} else {
			    // Aus dem Netzwerk holen!!!
				NSString *myFileName = [[fileArray objectAtIndex:[filePicker selectedRowInComponent:0]] retain];
				
				//NSString *fileURL = [NSString stringWithFormat:@"%@/%@",serverTextField.text ,myFileName ];
				
				NSString *fileURL = [self serverURLforSelectedFile:[filePicker selectedRowInComponent:0]];
				[self fetchDatabaseFromServer:fileURL andDatabaseName:myFileName withUserName:nil andPassword:nil];
				//NSLog(@"Fetch remote Database: %@", fileURL);
				
				////// kf check start //
				//NSLog(@"Lokale Datei %@ :", myFileName);
				////// kf check start //
				
				NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
				NSString *documentsDirectory = [paths objectAtIndex:0];
				
				NSString *path = [documentsDirectory stringByAppendingPathComponent:myFileName];				
				
				[self openDatabase:path andDatabaseName:myFileName withPassword:textFieldString];
				//NSLog(@"and open local Database: %@", path);
			}
		} else if ( [alertType isEqualToString:@"DATABASE"]) { 
			NSString *fileURL = [NSString stringWithFormat:@"%@/%@",serverTextField.text , textFieldString];
			//NSLog(@"Datenbank %@ herunterladen", fileURL);
			
			[self fetchDatabaseFromServer:fileURL andDatabaseName:textFieldString withUserName:nil andPassword:nil];
			
		} else if ( [alertType isEqualToString:@"DELETE"]) {
			NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
			NSString *documentsDirectory = [paths objectAtIndex:0];
			NSString *destinationFilename;
			destinationFilename = [[documentsDirectory stringByAppendingPathComponent:[fileArray objectAtIndex:[filePicker selectedRowInComponent:0]]] retain];
			
			NSFileManager *fileManager = [NSFileManager defaultManager];
			
			
			if ([fileManager fileExistsAtPath:destinationFilename]) {
				BOOL success = [fileManager removeItemAtPath:destinationFilename error:nil];
				if (success) {
					// Entferne Key aus fileList
					[fileList removeObjectForKey:[fileArray objectAtIndex:[filePicker selectedRowInComponent:0]]];
					// pList wieder sichern
					[fileList writeToFile:[documentsDirectory stringByAppendingPathComponent:iKeePassDatabaseFiles] atomically:YES];
				}
				//NSLog(@"Deleted with success %@", [success stringValue]);
			}
			[self reloadFilePicker];
		}
	}
}

/*- (BOOL)isNetworkAvailable:(NSString* )databaseAtServerURL
{  
	SCNetworkReachabilityFlags flags;
	BOOL receivedFlags;
	BOOL bRet = FALSE ;
	
	SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithName(CFAllocatorGetDefault(), [databaseAtServerURL UTF8String]);
	receivedFlags = SCNetworkReachabilityGetFlags(reachability, &flags);
	CFRelease(reachability);

	if (!receivedFlags || (flags == 0) )
	{
		// internet not available
		bRet = FALSE ;
	} else {
		// internet available
		bRet = TRUE ;
	}
	
	return bRet ;
}
*/

-(void) fetchDatabaseFromServer:(NSString* )databaseAtServerURL andDatabaseName:(NSString *)databaseName withUserName:(NSString*) userName andPassword:(NSString *) password {
	// HUD einblenden mit Downloadhinweis
	
	[self performSelector:@selector(presentSheet)];
	
	// Check, if a network is reachable start //
	//BOOL bNetWorkAvailable = FALSE ;
	SCNetworkReachabilityFlags flags;
	BOOL receivedFlags;
	SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithName(CFAllocatorGetDefault(), [databaseAtServerURL UTF8String]);
	receivedFlags = SCNetworkReachabilityGetFlags(reachability, &flags);
	CFRelease(reachability);
	
	//if (isNetworkAvailable(databaseAtServerURL) == FALSE)
	if (!receivedFlags || (flags == 0) )
	{
		// kill wait cursor //
		[self performSelector:@selector(killHUD:) withObject:HUD afterDelay:1.0];

		UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"iKeePass", @"") message:NSLocalizedString(@"ERROR_NONETWORK", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil, nil];
		[myAlertView show];
		[myAlertView release];
		return ;
	}
	// Check, if a network is reachable end //
	
	 // synchronous download of the keepass database file.
	 // todo: error handling, timeout and much more!
	NSURLRequest *request = [[NSURLRequest alloc] initWithURL:[[NSURL alloc ] initWithString:databaseAtServerURL]];
	NSHTTPURLResponse *response = NULL;
	NSError *error= NULL;
//	NSData *data = [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:nil] ;
	
	NSData *data = [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:&error] ;

	if (error != NULL)
	{
		// kill wait cursor //
		[self performSelector:@selector(killHUD:) withObject:HUD afterDelay:1.0];

		UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"iKeePass", @"") message:NSLocalizedString(@"COULD_NOT_LOAD_DB_MESSAGE", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil, nil];
		[myAlertView show];
		[myAlertView release];
		return ;
	}
	
	////////////////////
	// for testing only //
	////data = NULL ;
	////////////////////
	
	if (data == NULL)
	{
			// kill wait cursor //
			[self performSelector:@selector(killHUD:) withObject:HUD afterDelay:1.0];
			
			UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"iKeePass", @"") message:NSLocalizedString(@"COULD_NOT_LOAD_DB_MESSAGE", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil, nil];
			[myAlertView show];
			[myAlertView release];
			return ;
	}

	if ([data bytes] == 0)
	{
			// kill wait cursor //
			[self performSelector:@selector(killHUD:) withObject:HUD afterDelay:1.0];

			UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"iKeePass", @"") message:NSLocalizedString(@"COULD_NOT_LOAD_DB_MESSAGE2", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil, nil];
			[myAlertView show];
			[myAlertView release];
			return ;
	}
	
	// kf added 2009-06-06 start //
	if( [response isKindOfClass:[NSHTTPURLResponse class]] ) 
	{
		//NSLog(@"response is an http response");
		NSInteger rc = [((NSHTTPURLResponse*) response) statusCode];
		//NSLog(@"status code: %d", rc);
	
//		if (rc == 300)
//		{
//			UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"iKeePass", @"") message:NSLocalizedString(@"COULD_NOT_LOAD_DB_MESSAGE2", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil, nil];
//			[myAlertView show];
//			[myAlertView release];
//			return ;
//		}
	} 
	else
	{
		if (error != NULL)
		{
			// kill wait cursor //
			[self performSelector:@selector(killHUD:) withObject:HUD afterDelay:1.0];

			UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"iKeePass", @"") message:NSLocalizedString(@"COULD_NOT_LOAD_DB_MESSAGE", @"") delegate:self cancelButtonTitle:NSLocalizedString(@"CANCEL_BUTTON", @"") otherButtonTitles:nil, nil];
			[myAlertView show];
			[myAlertView release];
			return ;
		}
	}	

	
//	NSString *plainString = [NSString stringWithCString:[data bytes]];	
//	NSLog(@"URL wrong!: Data: %@", plainString);		
	///NSString *responseDataString = [[NSString alloc] initWithData:responseData encoding:NSASCIIStringEncoding];
	//NSLog(@"Response from Google: %@", responseDataString);
	
	//if ([error code])
	//{
	//	NSLog(@"%@ %d", [error domain], [error code] );	
	//}
	
	// kf added 2009-06-06 end //

		
	// write the downloaded data into a database file in the user home directory //
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	NSString *destinationFilename;
	
	destinationFilename = [[documentsDirectory stringByAppendingPathComponent:databaseName] retain];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	
	if ([fileManager fileExistsAtPath:destinationFilename]) {
		BOOL success = [fileManager removeItemAtPath:destinationFilename error:nil];
		//NSLog(@"Deleted with success %@", [success stringValue]);
	}
	 //NSLog(@"Tmp File Name: %@",destinationFilename);
	[data writeToFile:destinationFilename atomically:YES] ;
	
	// kill wait cursor //
	[self performSelector:@selector(killHUD:) withObject:HUD afterDelay:1.0];
	// Servernamen im Resoruce Fork sichern!!
	//NSLog(@"try to save servername % @ in Resource Fork %@",databaseAtServerURL ,[destinationFilename stringByAppendingPathComponent:@"rsrc"] );
	//[databaseAtServerURL writeToFile:[destinationFilename stringByAppendingPathComponent:@"rsrc"] atomically:NO];
	[fileList setValue:databaseAtServerURL forKey:databaseName];
	
	[fileList writeToFile:[documentsDirectory stringByAppendingPathComponent:iKeePassDatabaseFiles] atomically:YES];
	
	[self reloadFilePicker];
	
	
}


- (void) reloadFilePicker {
	// Array neu befüllen für Filepicker
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	NSFileManager *fileManager = [NSFileManager defaultManager];
	
	NSMutableArray *files = (NSMutableArray*)[[fileManager directoryContentsAtPath:documentsDirectory] pathsMatchingExtensions:[NSArray 
																									  arrayWithObjects:@"kdb", @"xml", nil]];
	[fileArray removeAllObjects];
	
	int i=0;
	for (i=0; i< files.count; i++) {
		//NSLog(@"Files %i: %@", i, [files objectAtIndex:i]);
		[fileArray addObject:[files objectAtIndex:i]];
		
	}
	 
	// Jetzt FilePicker neu laden!
	[filePicker reloadAllComponents];
	
	if (fileArray.count > 0) {
		[deleteButton setEnabled:YES];
		// Filepicker's erstes element auswählen
		//NSLog(@"FetchDatabase... Selektierte Zeile: %i", [filePicker selectedRowInComponent:0]);
		sizeLabel.text = [self fileInformationForSelectedFile:[filePicker selectedRowInComponent:0]];
	} else {
		[deleteButton setEnabled:NO];
	}
}

-(void) openDatabase:(NSString* )pathToDatabase andDatabaseName:(NSString *)databaseName withPassword:(NSString*) passwordForDatabase {
	//NSLog(@"Open Database File: %@ With Passwd: %@", pathToDatabase, passwordForDatabase);
	NSArray *tree;
	if ([pathToDatabase hasSuffix:@".xml"]) {
		XMLReader *reader = [[[XMLReader alloc] init] retain];
		dataBaseLoadedSuccessful = [reader parseXMLFileAtURL:pathToDatabase];
		tree = reader.tree;
	} else if ([pathToDatabase hasSuffix:@".kdb"]) {
		// Hier den KDB-Parser starten
		//NSLog(@"Erzeuge KDBReader");
		KDBReader *reader = [[[KDBReader alloc] init] retain];
		dataBaseLoadedSuccessful = [reader parseKDBFileAtURL:pathToDatabase withPassword:passwordForDatabase];
		//tree = reader.parseKDBFileAtURL(databaseName, passwordForDatabase);
		//NSLog(@"Open Datenbank %@ mit Passwort: %@", databaseName, passwordForDatabase);
		tree = reader.tree;
		
	} else {
		// Falsches Format, return
		return;
	}
	
	if (dataBaseLoadedSuccessful) {
		// Datenbank wurde gelesen
		RootViewController *rootController = [[[RootViewController alloc] initWithNibName:nil bundle:nil] retain];
		rootController.databaseName = databaseName;
		//[navigationController pushViewController:rootController animated:YES];
		NSMutableArray *viewControllers = [[NSMutableArray alloc] init];
		[viewControllers addObject:rootController];
	
		[navigationController setViewControllers:viewControllers];
		
		[[self window] addSubview:[navigationController view]];
	
		[[navigationController visibleViewController] setTree:tree];
	
		[[navigationController visibleViewController] setNavigationController:navigationController];
		//NSLog(@"Number of Viewontrollers in navigationController: %i", navigationController.viewControllers);
	} else { // Datenbank kpnnte nicht gelesen werden!
		//NSLog(@"Fehler beim öffnen der Datenbank");
	}
}


- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView {
	return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component {
	return fileArray.count;
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
	sizeLabel.text = [self fileInformationForSelectedFile:[pickerView selectedRowInComponent:0]];
}

- (NSString *) serverURLforSelectedFile: (NSInteger )selectedFileNum {
	// Nur wenn eine DB da ist!!
	if (selectedFileNum >= 0) {
		NSString *fileName = [fileArray objectAtIndex:selectedFileNum];
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString *documentsDirectory = [paths objectAtIndex:0];
		
		NSString *path = [documentsDirectory stringByAppendingPathComponent:fileName];
		
		NSString *serverURL = [NSString stringWithContentsOfFile:[path stringByAppendingPathComponent:@"rsrc"]];
		if (serverURL.length > 0)
			return serverURL;
		else {
			if ([fileList count] > 0)
				if ([fileList objectForKey:fileName] != nil)
					return [fileList objectForKey:fileName]; 
				else
					return [serverTextField.text stringByAppendingPathComponent:fileName];
			else
				return @"";
		}
	}
	return @"";
}


- (NSString *) fileInformationForSelectedFile:(NSInteger )selectedFileNum {
	// Nur wenn eine DB da ist!!
	if (selectedFileNum >= 0) {
		// report the selection to the UI label
		NSString *fileName = [fileArray objectAtIndex:selectedFileNum];
		//label.text = [NSString stringWithFormat:@"%@ - %d",
		//			  [pickerViewArray objectAtIndex:[pickerView selectedRowInComponent:0]], [pickerView selectedRowInComponent:1]];
		/*sizeLabel.text = [NSString stringWithFormat:@"%@",
		 [fileArray objectAtIndex:[pickerView selectedRowInComponent:0]]];
		 NSLog(@"Value Selected: %@" , sizeLabel.text);
		 */
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString *documentsDirectory = [paths objectAtIndex:0];
	
		NSString *path = [documentsDirectory stringByAppendingPathComponent:fileName];
	
		NSFileManager *fileManager = [NSFileManager defaultManager];
		NSDictionary *fileAttributes = [fileManager fileAttributesAtPath:path traverseLink:YES];
		if (fileAttributes != nil) {
			NSNumber *fileSize;
			NSString *fileOwner;
			NSDate *fileModDate;
			if (fileSize = [fileAttributes objectForKey:NSFileSize]) {
				//NSLog(@"File size: %qi\n", [fileSize unsignedLongLongValue]);
			}
			if (fileOwner = [fileAttributes objectForKey:NSFileOwnerAccountName]) {
				//NSLog(@"Owner: %@\n", fileOwner);
			}
			if (fileModDate = [fileAttributes objectForKey:NSFileModificationDate]) {
				//NSLog(@"Modification date: %@\n", fileModDate);
			}
			//return [NSString stringWithFormat:@"%@/%@ - %qi Byte",serverTextField.text, fileName, [fileSize unsignedLongLongValue]];
			return [NSString stringWithFormat:@"%@ - %qi Byte",[self serverURLforSelectedFile:selectedFileNum], [fileSize unsignedLongLongValue]];
		}
		else {
			//NSLog(@"Path (%@) is invalid.", path);
			return [NSString stringWithFormat:@"Path (%@) is invalid.", path];
		}
	}
	return @"-";
}


- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
	NSString *returnStr;
	if (row >=0) {
		
		if (component == 0)
		{
			returnStr = [fileArray objectAtIndex:row];
		}
		else
		{
			returnStr = [[NSNumber numberWithInt:row] stringValue];
		}
	} else {
		returnStr = @"-";
	}
	return returnStr;
}

- (CGFloat)pickerView:(UIPickerView *)pickerView widthForComponent:(NSInteger)component
{
	CGFloat componentWidth;
	if (component == 0)
		componentWidth = 240.0;	// first column size is wider to hold names
	else
		componentWidth = 40.0;	// second column is narrower to show numbers
	return componentWidth;
}

- (CGFloat)pickerView:(UIPickerView *)pickerView rowHeightForComponent:(NSInteger)component
{
	return 40.0;
}


// Implement loadView to create a view hierarchy programmatically.
- (void)loadView {
	[super loadView];
	dataBaseLoadedSuccessful = NO;
	editButton.title = NSLocalizedString(@"EDIT_BUTTON", @"");
	self.title = NSLocalizedString(@"AppTitle", @"iKeePass");
	serverLabel.text = NSLocalizedString(@"ServerTitle", @"");
	useLocalDBLabel.text = NSLocalizedString(@"UseLocalDatabase", @"");
	sizeLabel.text = @"-";
	openButton.title = NSLocalizedString(@"OpenButtonTitle", @"");
	toggleServerField = NO;
	[navigationController setTitle:NSLocalizedString(@"AppTitle", @"iKeePass")];
	
	// Serverfeld mit default wert aus NSUserDefaults setzten!
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults]; 
	NSString *serverURL = [defaults stringForKey:@"IKEEPASS_SERVER_URL"];
	if (serverURL == nil)
		serverURL = @"http://www.ikeepass.de";
	serverTextField.text = serverURL;
	serverTextField.clearsOnBeginEditing = NO;
	serverTextField.clearButtonMode = UITextFieldViewModeWhileEditing;
	serverTextField.autocorrectionType = UITextAutocorrectionTypeNo;
	
	
	fileArray = [[NSMutableArray alloc] init];
	
	// Files auslesen:
	NSFileManager *fileManager = [NSFileManager defaultManager];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	
	// plist NSDictionary *fileList aus File iKeePassDatabaseFiles.plist laden!!
	fileList = [[NSMutableDictionary 
						   dictionaryWithContentsOfFile:[documentsDirectory stringByAppendingPathComponent: iKeePassDatabaseFiles]] retain]; 
	if (!fileList)
		fileList = [[[NSMutableDictionary alloc] init] retain];
	
	
	NSMutableArray *files = (NSMutableArray*)[[fileManager directoryContentsAtPath:documentsDirectory] pathsMatchingExtensions:[NSArray 
																																arrayWithObjects:@"kdb", @"xml", nil]];
	int i=0;
	for (i=0; i< files.count; i++) {
		//NSLog(@"Files %i: %@", i, [files objectAtIndex:i]);
		[fileArray addObject:[files objectAtIndex:i]];
		
	}
		
	[filePicker setDelegate:self];
	[filePicker setNeedsDisplay];
	
	// Filepicker's erstes element auswählen
	if (fileArray.count > 0) {
		//NSLog(@"Load View .. Selektierte Zeile: %i", [filePicker selectedRowInComponent:0]);
		sizeLabel.text = [self fileInformationForSelectedFile:[filePicker selectedRowInComponent:0]];
		[deleteButton setEnabled:YES];
	}
	else
	{
		// kf added //
		[fileArray addObject:[NSString stringWithFormat: @"test_database.kdb"]];
		//sizeLabel.text = [self fileInformationForSelectedFile:[filePicker selectedRowInComponent:0]];
		//fetchDatabaseFromServer() ;
		sizeLabel.text = @"http://www.ikeepass.de/test_database.kdb" ;	
		
		// download database to local iPhone //
		//NString fileURL = 
		[self fetchDatabaseFromServer:sizeLabel.text andDatabaseName:@"test_database.kdb" withUserName:nil andPassword:nil];
		
	}
}

- (IBAction )addDatabaseFromServer:(id) sender  {
	//NSLog(@"Datenbank von URL laden!");
	alertType = @"DATABASE";
	// Passwort abfragen!
	UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"DATABASE_NAME", @"Datenbankname:") message:NSLocalizedString(@"PLEASE_ENTER_DB_NAME_MESSAGE", @"\n\nBitte gebe Sie den Datenbanknamen ein:") delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"OK", nil];
	myTextField = [[UITextField alloc] initWithFrame:CGRectMake(12.0, 45.0, 260.0, 25.0)];
	[myTextField setBackgroundColor:[UIColor whiteColor]];
	//myTextField.secureTextEntry = YES;
	[myAlertView addSubview:myTextField];
	myTextField.autocorrectionType = UITextAutocorrectionTypeNo;
	CGAffineTransform myTransform = CGAffineTransformMakeTranslation(0.0, 130.0);
	[myAlertView setTransform:myTransform];
	
	[myAlertView show];
	[myAlertView release];
	
	[myTextField becomeFirstResponder];
	
}



- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data

	//UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"DATABASE_NAME", @"Datenbankname:") message:NSLocalizedString(@"PLEASE_ENTER_DB_NAME_MESSAGE", @"\n\nBitte gebe Sie den Datenbanknamen ein:") delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"OK", nil];
}


- (void)dealloc {
    [super dealloc];
}

@end
