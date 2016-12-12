#import <Foundation/Foundation.h>
#import <xar/xar.h>
#import "XarBase.h"

@interface XarSubdoc : XarBase
{
  xar_subdoc_t xarSubdoc;
}

+ (id)xarSubdocWithSubdoc:(xar_subdoc_t)subdoc;
- (id)initWithSubdoc:(xar_subdoc_t)subdoc;

- (xar_subdoc_t)subdoc;

@end
