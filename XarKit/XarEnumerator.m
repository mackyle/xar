#import "XarEnumerator.h"
#import "XarArchive.h"
#import "XarFile.h"
//#import "XarSubdoc.h"

enum {
	XarFileEnumerator,
	XarPropEnumerator,
	XarSubdocEnumerator,
};

@interface XarEnumerator (PRIVATE)
- (id)initWithObject:(id)archive type:(unsigned)type;
@end

@implementation XarEnumerator

+ (id)fileEnumeratorWithArchive:(XarArchive *)archive
{
	return [[[self alloc] initWithObject:archive type:XarFileEnumerator] autorelease];
}

+ (id)propEnumeratorWithFile:(XarFile *)file
{
	return [[[self alloc] initWithObject:file type:XarPropEnumerator] autorelease];
}

+ (id)subdocEnumeratorWithArchive:(XarArchive *)archive
{
	return [[[self alloc] initWithObject:archive type:XarSubdocEnumerator] autorelease];
}

- (id)initWithObject:(id)object type:(unsigned)type
{
	if ((self = [super init])) {
		enumeratorType = type;

		switch (enumeratorType) {
			case XarFileEnumerator:
				NSAssert([object isKindOfClass:[XarArchive class]], @"object/type mismatch");

				if ([object xar]) {
					iterator = xar_iter_new();
					if (iterator) {
						currentFile = xar_file_first([object xar], iterator);
					}
				}
				break;
			case XarPropEnumerator:
				NSAssert([object isKindOfClass:[XarFile class]], @"object/type mismatch");

				if ([object file]) {
					iterator = xar_iter_new();
					if (iterator) {
						currentProp = xar_prop_first([object file], iterator);
					}
				}
				break;
			case XarSubdocEnumerator:
				NSAssert([object isKindOfClass:[XarArchive class]], @"object/type mismatch");

				if ([object xar]) {
					currentSubdoc = xar_subdoc_first([object xar]);
				}
				break;
		}
	}

	return self;
}

- (void)dealloc
{
	xar_iter_free(iterator);
	[super dealloc];
}

- (id)nextObject
{
	id ret;
	xar_file_t file;
	const char *prop;
	xar_subdoc_t subdoc;

	switch (enumeratorType) {
		case XarFileEnumerator:
			file = currentFile;
			currentFile = xar_file_next(iterator);
			ret = [XarFile xarFileWithFile:file];
			break;
		case XarPropEnumerator:
			prop = currentProp;
			currentProp = xar_prop_next(iterator);
			ret = [NSString stringWithUTF8String:prop];
			break;
		case XarSubdocEnumerator:
			subdoc = currentSubdoc;
			currentSubdoc = xar_subdoc_next(currentSubdoc);
			//ret = [XarSubdoc xarSubdocWithSubdoc:subdoc];
			break;
	}

	return ret;
}

- (NSArray *)allObjects
{
	NSMutableArray *array = [NSMutableArray new];
	xar_file_t file;
	const char *prop;
	xar_subdoc_t subdoc;

	switch (enumeratorType) {
		case XarFileEnumerator:
			for (file = currentFile; file; file = xar_file_next(iterator)) {
				[array addObject:[XarFile xarFileWithFile:file]];
			}
			break;
		case XarPropEnumerator:
			for (prop = currentProp; prop; prop = xar_prop_next(iterator)) {
				[array addObject:[NSString stringWithUTF8String:prop]];
			}
			break;
		case XarSubdocEnumerator:
			for (subdoc = currentSubdoc; subdoc; subdoc = xar_subdoc_next(subdoc)) {
				//[array addObject:[XarSubdoc xarSubdocWithSubdoc:subdoc];
			}
			break;
	}

	return [array autorelease];
}

@end
