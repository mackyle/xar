#import <Foundation/Foundation.h>
#import <xar/xar.h>

extern NSString *XarErrorSeverity;
extern NSString *XarErrorError;
extern NSString *XarErrorFile;
extern NSString *XarErrorString;
extern NSString *XarErrorErrno;

@class XarFile;

@interface XarArchive : NSObject
{
	xar_t xar;
	NSMutableArray *errors;
}

+ (id)archiveForReadingAtPath:(NSString *)path;
+ (id)archiveForWritingAtPath:(NSString *)path;

- (void)closeArchive;

- (xar_t)xar;

- (NSEnumerator *)fileEnumerator;

- (XarFile *)addPath:(NSString *)path;

- (BOOL)extractFile:(XarFile *)file toDirectory:(NSString *)path error:(NSArray **)errorPtr;

@end
