#import "XarFile.h"
#import "XarEnumerator.h"

@implementation XarFile

#pragma mark Initialization

+ (id)xarFileWithFile:(xar_file_t)file
{
	return [[[self alloc] initWithFile:file] autorelease];
}

- (id)initWithFile:(xar_file_t)file
{
	if ((self = [super init])) {
		if (file) {
			xarFile = file;
		} else {
			[self release];
			self = nil;
		}
	}

	return self;
}

#pragma mark Properties (KVC)

- (void)setValue:(id)value forKey:(NSString *)key
{
	xar_prop_set(xarFile, [key UTF8String], [value UTF8String]);
}

- (id)valueForKey:(NSString *)key
{
	const char *value;
	xar_prop_get(xarFile, [key UTF8String], &value);
	return [NSString stringWithUTF8String:value];
}

#pragma mark Various

- (NSEnumerator *)propEnumerator
{
	return [XarEnumerator propEnumeratorWithFile:self];
}

- (NSString *)path
{
	return [NSString stringWithUTF8String:xar_get_path(xarFile)];
}

- (xar_file_t)file
{
	return xarFile;
}

@end
