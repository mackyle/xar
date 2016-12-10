#import "XarSubdoc.h"
#import "XarEnumerator.h"

@implementation XarSubdoc

+ (id)xarSubdocWithSubdoc:(xar_subdoc_t)subdoc
{
  return [[[self alloc] initWithSubdoc:subdoc] autorelease];
}

- (id)initWithSubdoc:(xar_subdoc_t)subdoc
{
  if ((self = [super initWithBase:(xar_base_t)subdoc])) {
    if (subdoc) {
      xarSubdoc = subdoc;
    } else {
      [self release];
      self = nil;
    }
  }

  return self;
}

- (void)setValue:(id)value forKey:(NSString *)key
{
  xar_prop_set((xar_base_t) xarSubdoc, [key UTF8String], [value UTF8String]);
}

- (id)valueForKey:(NSString *)key
{
  const char *value;
  xar_prop_get((xar_base_t) xarSubdoc, [key UTF8String], &value);
  return [NSString stringWithUTF8String:value];
}

- (NSEnumerator *)propEnumerator
{
  return [XarEnumerator propEnumeratorWith:self];
}

- (xar_subdoc_t)subdoc
{
  return xarSubdoc;
}

@end
