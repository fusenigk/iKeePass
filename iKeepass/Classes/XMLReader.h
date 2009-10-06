//
//  XMLReader.h
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


@interface XMLReader : NSObject {
	NSString *groupName;
	NSString *entryName;
	NSString *workingItem;
	NSMutableArray *tree;
	NSMutableDictionary *currentGroup;
	NSMutableDictionary *currentEntry;
	NSMutableArray *currentChildren;
	NSMutableArray *groupStack;
	NSMutableArray *entryStack;
	
	NSString *currentNodeName;
	NSMutableString *currentNodeContent;
	
	NSInteger numGroup;
	NSInteger numEntry;
}

@property (readwrite, retain) NSMutableArray *groupStack;
@property (readwrite, retain) NSMutableArray *entryStack;
@property (readwrite, retain) NSMutableArray *currentChildren;
@property (readwrite, retain) NSString *groupName;
@property (readwrite, retain) NSString *entryName;
@property (readwrite, retain) NSString *workingItem;
@property (readwrite, retain) NSMutableArray *tree;
@property (readwrite, retain) NSMutableDictionary *currentGroup;
@property (readwrite, retain) NSMutableDictionary *currentEntry;

- (void)parserDidStartDocument:(NSXMLParser *)parser;
- (id)parseXMLAtURL:(NSURL *)url toObject:(NSString *)aClassName parseError:(NSError **)error;
- (BOOL)parseXMLFileAtURL:(NSString *)URL; //URL is the file path (i.e. /Applications/MyExample.app/MyFile.xml)
- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict;
- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName;
- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string;

@end
