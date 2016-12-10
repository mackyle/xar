#import <Foundation/Foundation.h>
#import <xar/xar.h>

@interface XarSubdoc : NSObject
{
	xar_subdoc_t xarSubdoc;
}

+ (id)xarSubdocWithSubdoc:(xar_subdoc_t)subdoc;
- (id)initWithSubdoc:(xar_subdoc_t)subdoc;

- (xar_subdoc_t)subdoc;

@end
