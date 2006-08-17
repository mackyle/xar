#import "XarArchive.h"
#import "XarEnumerator.h"
#import "XarFile.h"

NSString *XarErrorSeverity = @"Severity";
NSString *XarErrorError = @"Error";
NSString *XarErrorFile = @"File";
NSString *XarErrorString = @"String";
NSString *XarErrorErrno = @"Errno";

@interface XarArchive (PRIVATE)
- (id)initWithPath:(NSString *)path flags:(int32_t)flags;
- (void)addErrorWithSeverity:(int32_t)sev error:(int32_t)err context:(xar_errctx_t)ctx;
@end

static int32_t
err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx)
{
	NSLog(@"callback");
	[(id)usrctx addErrorWithSeverity:sev error:err context:ctx];
	return 0;
}

@implementation XarArchive

+ (id)archiveForReadingAtPath:(NSString *)path
{
	return [[[self alloc] initWithPath:path flags:READ] autorelease];
}

+ (id)archiveForWritingAtPath:(NSString *)path
{
	return [[[self alloc] initWithPath:path flags:WRITE] autorelease];
}

- (id)initWithPath:(NSString *)path flags:(int32_t)flags
{
	if ((self = [super init])) {
		xar = xar_open([path UTF8String], flags);
		errors = [NSMutableArray new];

		if (xar) {
			NSLog(@"registering errhandler");
			xar_register_errhandler(xar, err_callback, self);
		} else {
			[self release];
			self = nil;
		}
	}

	return self;
}

- (void)closeArchive
{
	if (xar) {
		xar_close(xar);
		xar = NULL;
	}
}

- (void)dealloc
{
	[self closeArchive];
	[errors release];
	[super dealloc];
}

- (xar_t)xar
{
	return xar;
}

- (NSEnumerator *)fileEnumerator
{
	return [XarEnumerator fileEnumeratorWithArchive:self];
}

- (XarFile *)addPath:(NSString *)path
{
	return [XarFile xarFileWithFile:xar_add(xar, [path UTF8String])];
}

- (BOOL)extractFile:(XarFile *)file toDirectory:(NSString *)path error:(NSArray **)errorPtr;
{
	int32_t status;
	NSFileManager *manager = [NSFileManager defaultManager];

	NSString *oldPath = [manager currentDirectoryPath];

	[manager changeCurrentDirectoryPath:path];
	status = xar_extract(xar, [file file]);
	[manager changeCurrentDirectoryPath:oldPath];

	if (status == -1) {
		*errorPtr = [[errors copy] autorelease];
	} else {
		*errorPtr = nil;
	}

	[errors removeAllObjects];

	return YES;
}

- (void)addErrorWithSeverity:(int32_t)sev error:(int32_t)err context:(xar_errctx_t)ctx;
{
	NSDictionary *errorDict = [[NSDictionary alloc] initWithObjectsAndKeys:
		[NSNumber numberWithInt:sev], XarErrorSeverity,
		[NSNumber numberWithInt:err], XarErrorError,
		[XarFile xarFileWithFile:xar_err_get_file(ctx)], XarErrorFile,
		[NSString stringWithUTF8String:xar_err_get_string(ctx)], XarErrorString,
		[NSNumber numberWithInt:xar_err_get_errno(ctx)], XarErrorErrno,
		nil];

	[errors addObject:errorDict];
	[errorDict release];
}

@end
