#import "XarFile.h"
#import "XarEnumerator.h"

@implementation XarFile

+ (id)xarFileWithFile:(xar_file_t)file
{
  return [[[self alloc] initWithFile:file] autorelease];
}

- (id)initWithFile:(xar_file_t)file
{
  if ((self = [super initWithBase:(xar_base_t)file])) {
    if (file) {
      xarFile = file;
    } else {
      [self release];
      self = nil;
    }
  }

  return self;
}

- (void)setValue:(id)value forKey:(NSString *)key
{
  xar_prop_set((xar_base_t) xarFile, [key UTF8String], [value UTF8String]);
}

- (id)valueForKey:(NSString *)key
{
  const char *value;
  xar_prop_get((xar_base_t) xarFile, [key UTF8String], &value);
  return [NSString stringWithUTF8String:value];
}

- (NSEnumerator *)propEnumerator
{
  return [XarEnumerator propEnumeratorWith:self];
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
