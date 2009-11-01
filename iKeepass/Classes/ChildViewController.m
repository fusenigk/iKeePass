//
//  ChildViewController.m
//  TestKeePass
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

#import "ChildViewController.h"


@implementation ChildViewController

@synthesize navigationController;
@synthesize child;

#define LABEL_TAG 1 
#define VALUE_TAG 2 
#define FIRST_CELL_IDENTIFIER @"TrailItemCell" 
#define SECOND_CELL_IDENTIFIER @"RegularCell" 
#define FONT_SIZE 14.0f
#define CELL_CONTENT_WIDTH 320.0f
#define CELL_CONTENT_MARGIN 10.0f


- (id)initWithStyle:(UITableViewStyle)style {
    // Override initWithStyle: if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
    if (self = [super initWithStyle:UITableViewStyleGrouped]) {
    }
    return self;
}



// Implement viewDidLoad to do additional setup after loading the view.
- (void)viewDidLoad {
    //[super viewDidLoad];
	self.title = [child objectForKey:@"title"];
	crypted = YES;
	
	//[[self tableView] setStyle:UITableViewStyleGrouped];
	//myTableView = [[UITableView alloc] initWithFrame:[[UIScreen mainScreen] 
	//			applicationFrame] style:UITableViewStyleGrouped];
	
	myTableView = [[UITableView alloc] initWithFrame:self.view.bounds style:UITableViewStyleGrouped];	
	myTableView.delegate = self;
	myTableView.dataSource = self;
	
	[self.view addSubview: myTableView];
	
	UIToolbar *toolbar = [UIToolbar new];
	toolbar.barStyle = UIBarStyleDefault;
	
	CGRect frame = CGRectMake(0.0, 0.0, 94.0, 27.0);
	UISwitch *switchCtl = [[UISwitch alloc] initWithFrame:frame];
	[switchCtl addTarget:self action:@selector(leftViewAction:) forControlEvents:UIControlEventValueChanged];
	switchCtl.backgroundColor = [UIColor clearColor];
	UIBarButtonItem *customItem = [[UIBarButtonItem alloc] initWithCustomView:switchCtl];
	[switchCtl release];
	
	frame = CGRectMake(0.0, 0.0, 70.0, 20.0);
	UILabel *label = [[[UILabel alloc] initWithFrame:frame] autorelease];
	label.textAlignment = UITextAlignmentRight;
    label.text = NSLocalizedString(@"PASSWD", @"");
    label.font = [UIFont boldSystemFontOfSize:14.0];
    label.backgroundColor = [UIColor clearColor];
	label.textColor = [UIColor colorWithRed:76.0/255.0 green:86.0/255.0 blue:108.0/255.0 alpha:1.0];
	UIBarButtonItem *labelItem = [[UIBarButtonItem alloc] initWithCustomView:label];
	[label release];
	
	NSArray *items = [NSArray arrayWithObjects: labelItem, customItem, nil];
	toolbar.items = items;
	[customItem release];
	[labelItem release];
	
	// size up the toolbar and set its frame
	[toolbar sizeToFit];
	CGFloat toolbarHeight = [toolbar frame].size.height;
	CGRect mainViewBounds = self.view.bounds;
	[toolbar setFrame:CGRectMake(CGRectGetMinX(mainViewBounds),
								 CGRectGetMinY(mainViewBounds) + CGRectGetHeight(mainViewBounds) - (toolbarHeight * 2.0),
								 CGRectGetWidth(mainViewBounds),
								 toolbarHeight)];
	
	[self.view addSubview:toolbar];
	[toolbar release];	
	
}

- (void)leftViewAction:(id)sender
{
	UISwitch *switchCtl = (UISwitch*)sender;
	if (switchCtl.on)
	{
		crypted = NO;
		passVal.text = [child objectForKey:@"password"];
	}
	else
	{
		crypted = YES;
		passVal.text = @"********";
	}
}

- (NSString *) redisplayPassword:(NSString*) key {
	if (crypted) {
		return @"********";
		//passVal.text = ;
	} else {
		return key;
		passVal.text = [child objectForKey:@"password"];
	}
}	

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return 10;
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    /*static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:CellIdentifier] autorelease];
    }
    // Configure the cell
    return cell;*/
	
	/*BayAreaTrailsAppDelegate *delegate = [[UIApplication sharedApplication] 
										  delegate]; */
	/*NSArray *trails = [[[delegate bayAreaTrails] 
						objectAtIndex:delegate.currentRegion.row] 
					   objectForKey:@"Trails"]; */
	//NSDictionary *curTrail = [trails objectAtIndex:delegate.currentTrail.row]; 
	if (indexPath.section==0) { 
		UILabel *label, *value; 
		UITableViewCell *cell = [tableView 
								 dequeueReusableCellWithIdentifier:FIRST_CELL_IDENTIFIER]; 
		if (cell == nil) { 
			CGRect frame = CGRectMake(0, 0, 300, 44); 
			cell = [[[UITableViewCell alloc] initWithFrame:frame 
										   reuseIdentifier:FIRST_CELL_IDENTIFIER] 
					autorelease]; 
			cell.selectionStyle = UITableViewCellSelectionStyleNone; 
			label = [[[UILabel alloc] initWithFrame:CGRectMake(0.0, 12.0, 95.0, 
															   25.0)] autorelease]; 
			label.tag = LABEL_TAG; 
			label.font = [UIFont systemFontOfSize:12.0]; 
			label.textAlignment = UITextAlignmentRight; 
			label.textColor = [UIColor blueColor]; 
			label.autoresizingMask = UIViewAutoresizingFlexibleRightMargin | 
			UIViewAutoresizingFlexibleHeight; 
			[cell.contentView addSubview:label]; 
			value = [[[UILabel alloc] initWithFrame:CGRectMake(100.0, 12.0, 220.0, 
															   25.0)] autorelease]; 
			value.tag = VALUE_TAG; 
			value.font = [UIFont systemFontOfSize:12.0]; 
			value.textColor = [UIColor blackColor]; 
			value.lineBreakMode = UILineBreakModeWordWrap; 
			value.autoresizingMask = UIViewAutoresizingFlexibleWidth | 
			UIViewAutoresizingFlexibleHeight;
			if (indexPath.row != 5 )
				[cell.contentView addSubview:value]; 
		} else { 
			label = (UILabel *)[cell.contentView viewWithTag:LABEL_TAG]; 
			value = (UILabel *)[cell.contentView viewWithTag:VALUE_TAG]; 
		} 
		if (indexPath.row == 0) { 
			label.text = NSLocalizedString(@"TITLE", @""); 
			value.text = [child objectForKey:@"title"]; 
		} else if (indexPath.row == 1 ) { 
			label.text = NSLocalizedString(@"USER", @""); 
			value.text = [child objectForKey:@"username"];
		} else if (indexPath.row == 2 ) { 
			label.text = NSLocalizedString(@"PASSWD", @""); 
			value.text = [self redisplayPassword:[child objectForKey:@"password"]];; 
			passVal = [value retain];
		} else if (indexPath.row == 3 ) { 
			label.text = NSLocalizedString(@"URL", @""); 
			value.text = [child objectForKey:@"url"]; 
			value.textColor = [UIColor blueColor];
		} else if (indexPath.row == 4 ) { 
			label.text = NSLocalizedString(@"COMMENT", @""); 
			value.text = [child objectForKey:@"comment"]; 
			
			//NSLog(@"Comment: %@", value.text);
		} else if (indexPath.row == 5 ) { 
			label.text = NSLocalizedString(@"ICON", @""); 
			
			CGRect rect = CGRectMake(100, (44 - 16) / 2.0, 16, 16);
			
			UIImageView *imageView = [[UIImageView alloc] initWithFrame:rect];
			//imageView.tag = IMAGE_TAG;
			imageView.image = [[UIImage imageNamed:[[child objectForKey:@"icon"] stringByAppendingString:@".png"] ] retain];
			[cell.contentView addSubview:imageView];
			[imageView release];
			//[cell.contentView addSubview:value];
			//value.text = [child objectForKey:@"icon"]; 
		} else if (indexPath.row == 6 ) { 
			label.text = NSLocalizedString(@"CREATION", @""); 
			value.text = [child objectForKey:@"creation"]; 
		} else if (indexPath.row == 7 ) { 
			label.text =NSLocalizedString(@"LASTACCESS", @""); 
			value.text = [child objectForKey:@"lastaccess"]; 
		} else if (indexPath.row == 8 ) { 
			label.text = NSLocalizedString(@"LASTMOD", @""); 
			value.text = [child objectForKey:@"lastmod"]; 
		} else { // TRAIL_RESTRICT_ROW 
			label.text =NSLocalizedString(@"EXPIRE", @""); 
			value.text = [child objectForKey:@"expire"]; 
		} 
		// mehrere Zeilen zulassen
		value.lineBreakMode = UILineBreakModeWordWrap;
		value.numberOfLines = 0;
		return cell; 
	
	}// etc. 
	
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath;
{
    NSString *text = @"";
	
	if (indexPath.row == 0) { 
		text = [child objectForKey:@"title"]; 
	} else if (indexPath.row == 1 ) { 
		text = [child objectForKey:@"username"];
	} else if (indexPath.row == 2 ) { 
		text = [self redisplayPassword:[child objectForKey:@"password"]];; 
	} else if (indexPath.row == 3 ) { 
		text = [child objectForKey:@"url"]; 
	} else if (indexPath.row == 4 ) { 
		text = [child objectForKey:@"comment"]; 
	} else if (indexPath.row == 5 ) { 
		text = NSLocalizedString(@"ICON", @""); 
		
	} else if (indexPath.row == 6 ) { 
		text = [child objectForKey:@"creation"]; 
	} else if (indexPath.row == 7 ) { 
		text = [child objectForKey:@"lastaccess"]; 
	} else if (indexPath.row == 8 ) { 
		text = [child objectForKey:@"lastmod"]; 
	} else { // TRAIL_RESTRICT_ROW 
		text = [child objectForKey:@"expire"]; 
	} 
	
	
	//NSString *text = [items objectAtIndex:[indexPath row]];
    // calculate the row height by calculating the text heigh
    CGSize constraint = CGSizeMake(CELL_CONTENT_WIDTH - (CELL_CONTENT_MARGIN * 2), 20000.0f);
    CGSize size = [text sizeWithFont:[UIFont systemFontOfSize:FONT_SIZE] constrainedToSize:constraint lineBreakMode:UILineBreakModeWordWrap];
    CGFloat height = MAX(size.height, 40.0f);
    return height + (CELL_CONTENT_MARGIN * 0.9);
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {

	//printf("Selected: %d:%d", indexPath.row, indexPath.section) ;

	if (indexPath.section == 0 && indexPath.row == 1)
	{
		// click on username field
		// copy username to clipboard, with no time out !
		NSString *username = @"" ;
		UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:FIRST_CELL_IDENTIFIER]; 
		username =  [child objectForKey:@"username"];
		UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
		[pasteboard setValue: username forPasteboardType: @"public.utf8-plain-text"];		
	}
	else
	if (indexPath.section == 0 && indexPath.row == 2)
	{
		// click on password field
		// copy password to clipboard, with no time out !
		UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:FIRST_CELL_IDENTIFIER]; 			
		NSString *password = @"" ;
		password =  [child objectForKey:@"password"];
		UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
		[pasteboard setValue: password forPasteboardType: @"public.utf8-plain-text"];		
	}
	else
	if (indexPath.section == 0 && indexPath.row == 3)
	{
		// click on URL field
		// copy password to clipboard, with no time out !
		UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:FIRST_CELL_IDENTIFIER]; 
		NSString *webLink =  [child objectForKey:@"url"];

		NSString *password = @"" ;
		password =  [child objectForKey:@"password"];
		UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
		[pasteboard setValue: password forPasteboardType: @"public.utf8-plain-text"];		
		
		[[UIApplication sharedApplication] openURL:[NSURL URLWithString:webLink]];
	}
	else
	if (indexPath.section == 0 && indexPath.row == 4)
	{
		// click on comment field
		// copy password to clipboard, with no time out !
		UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:FIRST_CELL_IDENTIFIER]; 
			
		NSString *comment = @"" ;
		comment =  [child objectForKey:@"comment"];
		UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
		[pasteboard setValue: comment forPasteboardType: @"public.utf8-plain-text"];		
	}
}

/*
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
    }
    if (editingStyle == UITableViewCellEditingStyleInsert) {
    }
}
*/

/*
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    return YES;
}
*/

/*
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/

/*
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    return YES;
}
*/


/*
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}
*/
/*
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}
*/
/*
- (void)viewWillDisappear:(BOOL)animated {
}
*/
/*
- (void)viewDidDisappear:(BOOL)animated {
}
*/
/*
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}
*/

- (void)dealloc {
    [super dealloc];
}


@end

