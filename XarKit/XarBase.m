#import <Foundation/Foundation.h>
#import <xar/xar.h>
#import "XarBase.h"

@implementation XarBase
+ (id)xarBaseWithBase:(xar_base_t)base
{
  return [[[self alloc] initWithBase:base] autorelease];
}

- (id)initWithBase:(xar_base_t)base
{
  xarBase = base;
  return self;
}

-(xar_base_t) base
{
  return xarBase;
}
@end
