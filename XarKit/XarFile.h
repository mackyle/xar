#import <Foundation/Foundation.h>
#import <xar/xar.h>

@interface XarFile : NSObject
{
	xar_file_t xarFile;
}

+ (id)xarFileWithFile:(xar_file_t)file;
- (id)initWithFile:(xar_file_t)file;

- (NSString *)path;

- (xar_file_t)file;

@end
