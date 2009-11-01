//
//  RootViewController.m
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

#import "RootViewController.h"
#import "iKeePassAppDelegate.h"

#define ROW_HEIGHT 48

@implementation RootViewController

@synthesize tree;
@synthesize group;
@synthesize navigationController;
@synthesize databaseName;

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    //return 0;
	return tree.count;
}

#define NAME_TAG 1
#define TIME_TAG 2
#define IMAGE_TAG 3

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        //cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:CellIdentifier] autorelease];
		cell = [self tableviewCellWithReuseIdentifier:CellIdentifier];
	}
    
    // Set up the cell
	NSDictionary* value = [tree objectAtIndex:indexPath.row];
	if ([[value objectForKey:@"group"] boolValue]) {
		// this is a password group entry
		//cell.text = [value objectForKey:@"name"];
		UILabel *label;
		
		// Get the time zone wrapper for the row
		label = (UILabel *)[cell viewWithTag:NAME_TAG];
		label.text = [value objectForKey:@"title"];
		//NSLog(@"Label 1: %@", label.text);

		// Get the time zone wrapper for the row
		UIImageView *imageView = (UIImageView *)[cell viewWithTag:IMAGE_TAG];
		NSString *image = [value objectForKey:@"icon"];
		imageView.image = [[UIImage imageNamed:[ image stringByAppendingString:@".png"] ] retain];
		
		//NSDictionary* value = [tree objectAtIndex:indexPath.row];
		//if ([[value objectForKey:@"group"] boolValue]) {
		if ([[value objectForKey:@"children"] count] > 0)
				cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		//} 
	    
	
	} else {
		// this is a normal password entry
		//cell.text = [value objectForKey:@"title"];
		UILabel *label;
		
		// Get the time zone wrapper for the row
		label = (UILabel *)[cell viewWithTag:NAME_TAG];
		label.text = [value objectForKey:@"title"];
		//NSLog(@"Label 2: %@", label.text);
		
		// Get the time zone wrapper for the row
		UIImageView *imageView = (UIImageView *)[cell viewWithTag:IMAGE_TAG];
		NSString *image = [value objectForKey:@"icon"];
		imageView.image = [[UIImage imageNamed:[image stringByAppendingString:@".png"] ] retain]; 
		
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
	}
	
	
    return cell;
}



- (UITableViewCell *)tableviewCellWithReuseIdentifier:(NSString *)identifier {
	
	/*
	 Create an instance of UITableViewCell and add tagged subviews for the name, local time, and quarter image of the time zone.
	 */
	CGRect rect;
	
	rect = CGRectMake(0.0, 0.0, 320.0, ROW_HEIGHT);
	
	UITableViewCell *cell = [[[UITableViewCell alloc] initWithFrame:rect reuseIdentifier:identifier] autorelease];
	
#define LEFT_COLUMN_OFFSET 10.0
#define LEFT_COLUMN_WIDTH  30.0
	
#define MIDDLE_COLUMN_OFFSET 50.0
#define MIDDLE_COLUMN_WIDTH 310.0
	
#define MAIN_FONT_SIZE 18.0
#define LABEL_HEIGHT 26.0
	
#define IMAGE_SIDE 30.0
	
	/*
	 Create labels for the text fields; set the highlight color so that when the cell is selected it changes appropriately.
	 */
	UILabel *label;
	
	// Create an image view for the quarter image
	
	rect = CGRectMake(LEFT_COLUMN_OFFSET, (ROW_HEIGHT - IMAGE_SIDE) / 2.0, LEFT_COLUMN_WIDTH, IMAGE_SIDE);
	
	UIImageView *imageView = [[UIImageView alloc] initWithFrame:rect];
	imageView.tag = IMAGE_TAG;
	[cell.contentView addSubview:imageView];
	[imageView release];
	
	
	rect = CGRectMake(MIDDLE_COLUMN_OFFSET, (ROW_HEIGHT - LABEL_HEIGHT) / 2.0, MIDDLE_COLUMN_WIDTH, LABEL_HEIGHT);
	label = [[UILabel alloc] initWithFrame:rect];
	label.tag = NAME_TAG;
	label.font = [UIFont boldSystemFontOfSize:MAIN_FONT_SIZE];
	label.adjustsFontSizeToFitWidth = YES;
	[cell.contentView addSubview:label];
	label.highlightedTextColor = [UIColor whiteColor];
	[label release];
	
	return cell;
}


- (void)configureCell:(UITableViewCell *)cell forIndexPath:(NSIndexPath *)indexPath {
    
    /*
	 Cache the formatter. Normally you would use one of the date formatter styles (such as NSDateFormatterShortStyle), but here we want a specific format that excludes seconds.
	 */
	/*static NSDateFormatter *dateFormatter = nil;
	if (dateFormatter == nil) {
		dateFormatter = [[NSDateFormatter alloc] init];
		[dateFormatter setDateFormat:@"h:mm a"];
	}
	
	// Get the time zones for the region for the section
	Region *region = [displayList objectAtIndex:indexPath.section];
	NSArray *regionTimeZones = region.timeZoneWrappers;
	TimeZoneWrapper *wrapper = [regionTimeZones objectAtIndex:indexPath.row];
	
	UILabel *label;
	
	// Get the time zone wrapper for the row
	label = (UILabel *)[cell viewWithTag:NAME_TAG];
	label.text = wrapper.timeZoneLocaleName;
	
	// Get the time zone wrapper for the row
	[dateFormatter setTimeZone:wrapper.timeZone];
	label = (UILabel *)[cell viewWithTag:TIME_TAG];
	label.text = [dateFormatter stringFromDate:[NSDate date]];
	
	// Get the time zone wrapper for the row
	UIImageView *imageView = (UIImageView *)[cell viewWithTag:IMAGE_TAG];
	imageView.image = wrapper.image;*/
}    


- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    // Navigation logic -- create and push a new view controller
	NSDictionary* value = [tree objectAtIndex:indexPath.row];
	if ([[value objectForKey:@"group"] boolValue]) {
		// NSLog(@"Sollte Gruppe %@ anzeigen", [value objectForKey:@"title"]);
		
		//GroupViewController *groupViewController = [[[GroupViewController alloc] initWithNibName:nil bundle:nil] retain];
		RootViewController *groupViewController = [[[RootViewController alloc] initWithNibName:nil bundle:nil] retain];
		groupViewController.group = value;
		NSMutableArray *children = [value objectForKey:@"children"]; 
		//if (children != nil ) {
		if ([children count] > 0) {
			groupViewController.tree = children;
			groupViewController.navigationController = navigationController;
			[navigationController pushViewController:groupViewController animated:YES];
		}
		
	} else {
		// NSLog(@"Sollte Kind %@ öffnen", [value objectForKey:@"title"] );
		
		ChildViewController *childViewController = [[[ChildViewController alloc] initWithNibName:nil bundle:nil] retain];
		childViewController.child = value;
		childViewController.navigationController = navigationController;
		[navigationController pushViewController:childViewController animated:YES];
		
		//childViewController.title = [value objectForKey:@"title"];
	}
}

/***
 // kf removed !! //
- (UITableViewCellAccessoryType)tableView:(UITableView *)tableView accessoryTypeForRowWithIndexPath: (NSIndexPath *)indexPath {
	NSDictionary* value = [tree objectAtIndex:indexPath.row];
	if ([[value objectForKey:@"group"] boolValue]) {
		if ([[value objectForKey:@"children"] count] > 0)
			return UITableViewCellAccessoryDisclosureIndicator;
	} 
}
 */




- (void)viewDidLoad {
    [super viewDidLoad];
    // Uncomment the following line to add the Edit button to the navigation bar.
	//self.navigationItem.rightBarButtonItem = self.editButtonItem;
	//self.title = @"MSKeePass";
	if (group == NULL) {
		// Datenbankname als Title setzen
		self.title = databaseName;
		//self.title = @"iKeePass";
	} else {
		NSString *title = [NSString stringWithFormat:@"%@",[group objectForKey:@"title"]];
		self.title = title;
	}
	
}



/*
// Override to support editing the list
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:YES];
		[tree removeObjectAtIndex:indexPath.row];
    }   
    if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}*/




// Override to support conditional editing of the list
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return NO;
}




// Override to support rearranging the list
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
	if (fromIndexPath != toIndexPath) {
		if (fromIndexPath < toIndexPath) {
			// Wenn ein Element nach oben verschoben wird
			[tree insertObject:[tree objectAtIndex:fromIndexPath.row] atIndex:toIndexPath.row];
			[tree removeObjectAtIndex:fromIndexPath.row + 1];
		} else {
			// Wenn ein element nach unten verschoben wird
			[tree insertObject:[tree objectAtIndex:fromIndexPath.row] atIndex:toIndexPath.row];
			[tree removeObjectAtIndex:fromIndexPath.row - 1];

		}
	}
	[tableView setNeedsDisplay];
	[tableView setNeedsLayout];
	 
}




// Override to support conditional rearranging of the list
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return NO;
}


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

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}


- (void)dealloc {
    [super dealloc];
}


@end

