#import <Foundation/Foundation.h>
#import "XarEnumerator.h"
#import <xar/xar.h>

@interface XarBase : NSObject
{
  xar_base_t xarBase;
}

+(id) xarBaseWithBase:(xar_base_t) base;
-(id) initWithBase:(xar_base_t) base;
-(xar_base_t)base;
@end
