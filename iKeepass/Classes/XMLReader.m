//
//  XMLReader.m
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

#import "XMLReader.h"
#import <Foundation/Foundation.h>
#import <Foundation/NSXMLParser.h>

@implementation XMLReader

@synthesize groupStack;
@synthesize entryStack;

@synthesize currentChildren;
@synthesize groupName;
@synthesize entryName;
@synthesize tree;
@synthesize currentGroup;
@synthesize currentEntry;
@synthesize workingItem;

-(id)init {
	if (self = [super init]) {
		// NSLog(@"XMLReader init");
		numGroup = 0;
		numEntry = 0;
	}
	return self;
}

- (void)parserDidStartDocument:(NSXMLParser *)parser{	
	// NSLog(@"found file and started parsing");
}

- (BOOL)parseXMLFileAtURL:(NSString *)URL //URL is the file path (i.e. /Applications/MyExample.app/MyFile.xml)
{	
	//you must then convert the path to a proper NSURL or it won't work
	NSURL *xmlURL = [NSURL fileURLWithPath:URL];
	
	
	[groupStack release];
	groupStack = [[[NSMutableArray alloc] init] retain];
	/*[entryStack release];
	entryStack = [[NSMutableArray alloc] init];*/
	entryStack = groupStack;
	
	tree = [[NSMutableArray alloc] init];

	// here, for some reason you have to use NSClassFromString when trying to alloc NSXMLParser, otherwise you will get an object not found error
	// this may be necessary only for the toolchain
	NSXMLParser *parser = [[ NSClassFromString(@"NSXMLParser") alloc] initWithContentsOfURL:xmlURL];
	
	// Set self as the delegate of the parser so that it will receive the parser delegate methods callbacks.
	[parser setDelegate:self];
	
	// Depending on the XML document you're parsing, you may want to enable these features of NSXMLParser.
	[parser setShouldProcessNamespaces:YES];
	[parser setShouldReportNamespacePrefixes:YES];
	[parser setShouldResolveExternalEntities:YES];
	
	[parser parse];
	
	[parser release];
	return YES;
}

- (id)parseXMLAtURL:(NSURL *)url 
		   toObject:(NSString *)aClassName 
		 parseError:(NSError **)error
{
	[groupStack release];
	groupStack = [[NSMutableArray alloc] init];
	
	groupName = aClassName;
	
	NSXMLParser *parser = [[NSXMLParser alloc] initWithContentsOfURL:url];
	[parser setDelegate:self];
	
	[parser parse];
	
	if([parser parserError] && error) {
		*error = [parser parserError];
	}
	
	[parser release];
	
	return self;
}


- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict{	
	if (qName) {
        elementName = qName;
    }
	
	// NSLog(@"Open tag: %@", elementName);
	
	/*if ([elementName isEqualToString:@"group"]) {
		NSString *relAtt = [attributeDict valueForKey:@"group"];
		// NSLog(@"Group: %@", relAtt );
	}*/
	NSMutableDictionary *group;
	NSMutableDictionary *entry;
	if ([elementName isEqualToString:@"database"]) {
		// NSLog(@"didStart for elementName: %@", elementName);
	} else if ([elementName isEqualToString:@"group"]) {
		// NSLog(@"didStart for elementName: %@", elementName);
		
		// NSLog(@"Create Group Num %i", ++numGroup);
		
		// Erzeuge neue Gruppe
		group = [[[NSMutableDictionary alloc] init] retain];
		[group setObject:[NSNumber numberWithBool:YES] forKey:@"group"];
		
		// Erzeuge gleich das ChildrenArray
		NSMutableArray *children = [[[NSMutableArray alloc] init] retain];
		// Füge das Childrenarray zur Gruppe hinzu
		[group setObject:children forKey:@"children"];
		currentChildren = children;
		
		// Wenn der groupStack 0 ist, dann die Gruppe gleich dem tree hinzufügen!
		if ([groupStack count] == 0) {
			[tree addObject:group];
		} else {
			// Dem letzten element auf dem GroupStack die neue Gruppe in das childrenArray hinzufuegen
			[[[groupStack lastObject] objectForKey:@"children"] addObject:group];
		}
		// Auf jeden Fall auch dem groupStack hinzufügen!
		[groupStack addObject:group];
		
		currentGroup = group;
	} else if([elementName isEqualToString:@"entry"]) {
		// NSLog(@"didStart for elementName: %@", elementName);
		// NSLog(@"Create Entry Num %i", ++numEntry);
		
		// neues Dictionary erstellen!
		entry = [[[NSMutableDictionary alloc] init] retain];
		[entry setObject:[NSNumber numberWithBool:YES] forKey:@"entry"];
		// Eintrag auf den entryStack legen
		[entryStack addObject:entry];
		
		// Falls es keine Gruppe gibt, dann dem Tree hinzufügen!
		if (currentGroup == nil)
			[tree addObject:entry];
		else {
			// childrenArray holen und Entry hinzufügen!!!
			[[currentGroup objectForKey:@"children"] addObject:entry];
		}
		/*if ([workingItem isEqualToString:@"group"]) {
		 [group setObject:currentNodeContent forKey:currentNodeName];
		 } else {
		 // workingItem = "entry"
		 [entry setObject:currentNodeContent forKey:currentNodeName];
		 }*/
	
	} else {
		currentNodeName = [elementName copy];
		currentNodeContent = [[[NSMutableString alloc] init] retain];
	}
	
	//NSString *myFirstName= [attributeDict valueForKey:qName];
	//// NSLog(@"found this element: %@ with elementName: %@ and qName: %@ anzahl Elemente: %i", elementName, myFirstName, qName, [attributeDict count]);
	
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName{ 
	if (qName) {
        elementName = qName;
    }
    
	if ([elementName isEqualToString:@"database"]) {
		// NSLog(@"Close Group Num %@", elementName);
	} else if ([elementName isEqualToString:@"group"]) {
		// NSLog(@"Close Group Num %i", numGroup--);
		[groupStack removeLastObject];
		if ([groupStack count] > 0)
			currentGroup = [groupStack lastObject];
	} else if([elementName isEqualToString:@"entry"]) {	
		// NSLog(@"Create Group Num %i", numEntry--);
		[entryStack removeLastObject];
	} else {
		//if([elementName isEqualToString:currentNodeName]) {
		// NSLog(@"SetObject: %@ forKey: %@", currentNodeContent, currentNodeName);
		
		//if ( [entryStack count] == 0) {
			[[groupStack lastObject] setObject:currentNodeContent forKey:currentNodeName];
		/*} else {
			[[entryStack lastObject] setObject:currentNodeContent forKey:currentNodeName];
		}*/
		
		[currentNodeContent release];
		currentNodeContent = nil;
		
		[currentNodeName release];
		currentNodeName = nil;
	}
	
	// NSLog(@"Close tag: %@", elementName);
	
	/*if ([elementName isEqualToString:@"group"]) {
		// NSLog(@"Group: %@", elementName );
	}*/
	
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string{
	if (string != nil)
		[currentNodeContent appendString:string];
	//// NSLog(@"String: %@", string );
}

@end
